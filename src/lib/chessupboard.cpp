/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <QBitArray>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include "bleconnection.h"
#include "chessupboard.h"

namespace {
    // https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.2.0/nrf/libraries/bluetooth_services/services/nus.html#nordic-uart-service-nus
    const QLatin1String RX_CHARACTERISTIC_UUID("{6E400002-B5A3-F393-E0A9-E50E24DCCA9E}");
    const QLatin1String TX_CHARACTERISTIC_UUID("{6E400003-B5A3-F393-E0A9-E50E24DCCA9E}");

    const uint8_t CMD_ASSISTANCE      = 0x10;
    const uint8_t CMD_OK              = 0x21;
    const uint8_t CMD_PROMOTION_OK    = 0x23;
    const uint8_t CMD_SET_STATE       = 0x66;
    const uint8_t CMD_GET_STATE       = 0x67;
    const uint8_t CMD_PROMOTION       = 0x97;
    const uint8_t CMD_SEND_MOVE       = 0x99;
    const uint8_t CMD_WIN             = 0xb6;
    const uint8_t CMD_SETTINGS        = 0xb9;
    const uint8_t RESP_MOVE_OK        = 0x22;
    const uint8_t RESP_PROMOTION_OK   = 0x23;
    const uint8_t RESP_OK             = 0x24;
    const uint8_t RESP_BOARD_STATE    = 0x67;
    const uint8_t RESP_PROMOTION      = 0x97;
    const uint8_t RESP_MOVE           = 0xa3;
    const uint8_t RESP_SET_STATE_OK   = 0xb1;
    const uint8_t RESP_TOUCH          = 0xb8;
    const uint8_t RESP_UNDO           = 0xbd;
    const uint8_t MODE_REMOTE         = 0x02;
    const uint8_t MODE_AI             = 0x01;
    const uint8_t MODE_LOCAL          = 0x05;
}

namespace Chessboard {

ChessUpBoard::ChessUpBoard(const BoardAddress& address, BleConnection *connection, QObject *parent) :
    RemoteBoard(address, parent),
    m_connection(connection)
{
    m_uartService = connection->createServiceObject(QBluetoothUuid(CHESSUP_SERVICE), this);
    connect(m_uartService, &QLowEnergyService::stateChanged, this, &ChessUpBoard::discoveryFinished);
    connect(m_uartService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
        if (characteristic == m_rxCharacteristic && !m_inAckLoop)
            readFromBoard(value);
    });
    connect(m_uartService, &QLowEnergyService::characteristicWritten, this, [this]() {
        m_writeAcked = true;
    });
    m_batteryService = connection->createServiceObject(QBluetoothUuid::ServiceClassUuid::BatteryService, this);
    connect(m_batteryService, &QLowEnergyService::stateChanged, this, &ChessUpBoard::discoveryFinished);
    m_batteryService->discoverDetails(QLowEnergyService::FullDiscovery);
    m_uartService->discoverDetails(QLowEnergyService::FullDiscovery);
}

void ChessUpBoard::discoveryFinished()
{
    if (m_uartService->state() != QLowEnergyService::RemoteServiceDiscovered ||
        m_batteryService->state() != QLowEnergyService::RemoteServiceDiscovered)
        return;
    qDebug("ChessUpBoard::discoveryFinished");
    m_txCharacteristic = m_uartService->characteristic(QBluetoothUuid(RX_CHARACTERISTIC_UUID));
    Q_ASSERT(m_txCharacteristic.isValid());
    m_rxCharacteristic = m_uartService->characteristic(QBluetoothUuid(TX_CHARACTERISTIC_UUID));
    Q_ASSERT(m_rxCharacteristic.isValid());
    Q_ASSERT(m_rxCharacteristic.properties() & QLowEnergyCharacteristic::Notify);
    m_batteryCharacteristic = m_batteryService->characteristic(QBluetoothUuid::CharacteristicType::BatteryLevel);
    Q_ASSERT(m_batteryCharacteristic.isValid());
    m_rxClientCharacteristicConfiguration = m_rxCharacteristic.clientCharacteristicConfiguration();
    Q_ASSERT(m_rxClientCharacteristicConfiguration.isValid());
    m_batteryClientCharacteristicConfiguration = m_batteryCharacteristic.clientCharacteristicConfiguration();
    Q_ASSERT(m_batteryClientCharacteristicConfiguration.isValid());
    m_batteryService->readCharacteristic(m_batteryCharacteristic);
    // Enable notifications.
#ifdef __linux__
    m_uartService->writeDescriptor(m_rxClientCharacteristicConfiguration, QByteArray::fromHex("0100"));
    m_batteryService->writeDescriptor(m_batteryClientCharacteristicConfiguration, QByteArray::fromHex("0100"));
#else
    uint8_t buf[] = {0x01, 0x00};
    m_uartService->writeDescriptor(m_rxClientCharacteristicConfiguration, QByteArray::fromRawData(reinterpret_cast<const char *>(buf), sizeof(buf)));
    m_batteryService->writeDescriptor(m_batteryClientCharacteristicConfiguration, QByteArray::fromRawData(reinterpret_cast<const char *>(buf), sizeof(buf)));
#endif
    emit m_connection->connected(this);
    sendInit();
}

void ChessUpBoard::readFromBoard(const QByteArray& data)
{
    qDebug("ChessUpBoard::readFromBoard(%s)", qPrintable(data.toHex(' ')));
    if (data.isEmpty())
        return;
    m_responseRead = true;
    switch (static_cast<uint8_t>(data[0])) {
    case RESP_OK:
        break;
    case RESP_SET_STATE_OK:
        // response after CMD_SET_STATE
        requestRemoteBoardState();
        break;
    case RESP_PROMOTION_OK:
        break;
    case RESP_MOVE_OK:
        break;
    case RESP_BOARD_STATE: {
        BoardState boardState = boardStateFromRemote(data);
        emit remoteBoardState(boardState);
        break;
    }
    case RESP_MOVE: {
        Q_ASSERT(data.size() == 6);
        QByteArray cmdOk = QByteArray::fromRawData(reinterpret_cast<const char *>(&CMD_OK), 1);
        m_writeAcked = false;
        m_inAckLoop = true;
        while (!m_writeAcked) {
            m_uartService->writeCharacteristic(m_txCharacteristic, cmdOk, QLowEnergyService::WriteWithResponse);
            QObject().thread()->usleep(25*1000);
            qApp->processEvents();
        }
        m_inAckLoop = false;
        bool newMove = data != m_previousMove;
        m_previousMove = data;
        if (newMove) {
            int fromCol = data[2];
            int fromRow = data[3];
            int toCol = data[4];
            int toRow = data[5];
            emit remoteMove(fromRow, fromCol, toRow, toCol);
        }
        break;
    }
    case RESP_PROMOTION: {
        Q_ASSERT(data.size() == 2);
        QByteArray cmdOk = QByteArray::fromRawData(reinterpret_cast<const char *>(&CMD_PROMOTION_OK), 1);
        m_writeAcked = false;
        m_inAckLoop = true;
        while (!m_writeAcked) {
            m_uartService->writeCharacteristic(m_txCharacteristic, cmdOk, QLowEnergyService::WriteWithResponse);
            QObject().thread()->usleep(25*1000);
            qApp->processEvents();
        }
        m_inAckLoop = false;
        Piece piece = Piece::Queen;
        switch (data[1]) {
        case 1:
            piece = Piece::Rook;
            break;
        case 2:
            piece = Piece::Knight;
            break;
        case 3:
            piece = Piece::Bishop;
            break;
        case 4:
            piece = Piece::Queen;
            break;
        default:
            Q_ASSERT(data[1] >= 1 && data[1] <= 4);
            piece = Piece::Queen;
            break;
        }
        emit remotePromotion(piece);
    }
    case RESP_UNDO:
        emit remoteUndo();
        break;
    }
}

void ChessUpBoard::sendCommand(uint8_t cmd, const QByteArray& payload)
{
    writeToBoard(QByteArray::fromRawData(reinterpret_cast<const char *>(&cmd), 1) + payload);
}

void ChessUpBoard::writeToBoard(const QByteArray& data)
{
    qDebug("ChessUpBoard::writeToBoard(%s)", qPrintable(data.toHex(' ')));
    m_responseRead = false;
    m_uartService->writeCharacteristic(m_txCharacteristic, data, QLowEnergyService::WriteWithResponse);
#ifndef __linux__
    if (static_cast<uint8_t>(data.at(0)) == CMD_GET_STATE ||
        static_cast<uint8_t>(data.at(0)) == CMD_SETTINGS ||
        static_cast<uint8_t>(data.at(0)) == CMD_PROMOTION) {
        qApp->processEvents();
        while (!m_responseRead) {
            QObject().thread()->usleep(25*1000);
            qApp->processEvents();
        }
    }
#endif
}

void ChessUpBoard::sendInit()
{
    requestRemoteBoardState();
}

BoardState ChessUpBoard::boardStateFromRemote(const QByteArray& data)
{
    BoardState ret;
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            uint8_t c = static_cast<uint8_t>(data[1 + row * 8 + col]);
            ColouredPiece piece;
            switch (c) {
            case 0x40:
                piece = ColouredPiece::None;
                break;
            case 0x00:
                piece = ColouredPiece::WhitePawn;
                break;
            case 0x01:
                piece = ColouredPiece::WhiteRook;
                break;
            case 0x02:
                piece = ColouredPiece::WhiteKnight;
                break;
            case 0x03:
                piece = ColouredPiece::WhiteBishop;
                break;
            case 0x04:
                piece = ColouredPiece::WhiteQueen;
                break;
            case 0x05:
                piece = ColouredPiece::WhiteKing;
                break;
            case 0x08:
                piece = ColouredPiece::BlackPawn;
                break;
            case 0x09:
                piece = ColouredPiece::BlackRook;
                break;
            case 0x0a:
                piece = ColouredPiece::BlackKnight;
                break;
            case 0x0b:
                piece = ColouredPiece::BlackBishop;
                break;
            case 0x0c:
                piece = ColouredPiece::BlackQueen;
                break;
            case 0x0d:
                piece = ColouredPiece::BlackKing;
                break;
            }
            ret[row][col] = piece;
        }
    }
    ret.activeColour = (data[65] == 0) ? Colour::White : Colour::Black;
    ret.whiteKingsideCastlingAvailable = (data[66] == 1);
    ret.whiteQueensideCastlingAvailable = (data[67] == 1);
    ret.blackKingsideCastlingAvailable = (data[68] == 1);
    ret.blackQueensideCastlingAvailable = (data[69] == 1);
    if (data[70] >= 0 && data[70] < 64)
        ret.enpassantTarget = Square(data[70] >> 3, data[70] & 0x7);
    else
        ret.enpassantTarget = Square();
    ret.halfMoveClock = data[71];
    ret.fullMoveCount = data[72];
    return ret;
}

void ChessUpBoard::requestRemoteBoardState()
{
    sendCommand(CMD_GET_STATE, QByteArray(1, 0));
}

void ChessUpBoard::setBoardState(const BoardState& state)
{
    m_previousMove.clear();
    QString fenString = state.toFenString();
    QString fenStringPieceColourCastlingEnPassant = fenString.section(' ', 0, 3);
    uint8_t countData[3];
    countData[0] = state.halfMoveClock & 0xff;
    countData[1] = (state.fullMoveCount >> 8) & 0xff;
    countData[2] = state.fullMoveCount & 0xff;
    QByteArray data = fenStringPieceColourCastlingEnPassant.toLatin1() + " " +
            QByteArray::fromRawData(reinterpret_cast<char *>(countData), 3);
    Q_ASSERT(data.length() <= 0xff);
    uint8_t lenData[1] = { (uint8_t) data.length() } ;
    QByteArray payload = QByteArray::fromRawData(reinterpret_cast<char *>(lenData), 1) + data;
    sendCommand(CMD_SET_STATE, payload);
}

void ChessUpBoard::requestNewGame(const Chessboard::GameOptions& gameOptions)
{
    uint8_t buf[] = { 0x05,
        0x00, // white player type
        0x06, // white assistance level
        0x00,
        0x00, // black player type
        0x06, // black assistance level
        0x00,
        0xff, // hint limit
        0x00, // white player remote
        0x00, // black player remote
    };
    sendCommand(CMD_SETTINGS, QByteArray::fromRawData(reinterpret_cast<const char *>(buf), sizeof(buf)));
    setBoardState(BoardState::newGame());
    setGameOptions(gameOptions);
}

void ChessUpBoard::requestMove(int fromRow, int fromCol, int toRow, int toCol)
{
    Q_ASSERT(fromRow >= 0 && fromRow < 8 && toRow >= 0 && toCol < 8);
    uint8_t data[2];
    data[0] = fromRow * 8 + fromCol ;
    data[1] = toRow * 8 + toCol;
    sendCommand(CMD_SEND_MOVE,
                QByteArray::fromRawData(reinterpret_cast<const char *>(data), sizeof(data)));
}

void ChessUpBoard::requestPromotion(Piece piece)
{
    uint8_t data[1];
    switch (piece) {
    case Piece::Rook:
        data[0] = 1;
        break;
    case Piece::Knight:
        data[0] = 2;
        break;
    case Piece::Bishop:
        data[0] = 3;
        break;
    case Piece::Queen:
        data[0] = 4;
        break;
    default:
        Q_ASSERT(piece == Piece::Rook || piece == Piece::Knight || piece == Piece::Bishop || piece == Piece::Queen);
        data[0] = 4;
        break;
    }
    sendCommand(CMD_PROMOTION,
                QByteArray::fromRawData(reinterpret_cast<const char *>(data), sizeof(data)));
}

void ChessUpBoard::setGameOptions(const Chessboard::GameOptions& gameOptions)
{
    uint8_t mode;
    if (gameOptions.white.playerType == Chessboard::PlayerType::Ai ||
        gameOptions.black.playerType == Chessboard::PlayerType::Ai) {
        mode = MODE_AI;
    } else if (gameOptions.white.playerLocation == Chessboard::PlayerLocation::LocalApp ||
               gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalApp) {
        mode = MODE_REMOTE;
    } else {
        mode = MODE_LOCAL;
    }
    uint8_t buf[] = {
        mode,
        static_cast<uint8_t>((gameOptions.white.playerType == Chessboard::PlayerType::Ai) ? 0x01 : 0x00),
        static_cast<uint8_t>((gameOptions.white.playerType == Chessboard::PlayerType::Ai) ? 0x12 : gameOptions.white.assistanceLevel), // white assistance level
        0x01,
        static_cast<uint8_t>((gameOptions.black.playerType == Chessboard::PlayerType::Ai) ? 0x01 : 0x00),
        static_cast<uint8_t>((gameOptions.black.playerType == Chessboard::PlayerType::Ai) ? 0x12 : gameOptions.black.assistanceLevel), // black assistance level
        0x01,
        0xff, // hint limit
        static_cast<uint8_t>((gameOptions.white.playerType != Chessboard::PlayerType::Ai &&
                              gameOptions.white.playerLocation == Chessboard::PlayerLocation::LocalApp) ? 0x01 : 0x00), // white player remote?
        static_cast<uint8_t>((gameOptions.black.playerType != Chessboard::PlayerType::Ai &&
                              gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalApp) ? 0x01 : 0x00), // black player remote?
    };
    QByteArray payload = QByteArray::fromRawData(reinterpret_cast<const char *>(buf), sizeof(buf));
    if ((gameOptions.white.playerType == Chessboard::PlayerType::Human &&
         gameOptions.white.playerLocation == Chessboard::PlayerLocation::LocalBoard) ||
        (gameOptions.black.playerType == Chessboard::PlayerType::Human &&
         gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalBoard)) {
        payload.append(static_cast<uint8_t>((gameOptions.black.playerType == Chessboard::PlayerType::Human &&
                                             gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalBoard) ? 0x01 : 0x00));
    }
    sendCommand(CMD_SETTINGS, payload);
}

void ChessUpBoard::requestResignation(Colour requestor)
{
    uint8_t data[1];
    data[0] = (invertColour(requestor) == Colour::White) ? static_cast<uint8_t>(0x00) : static_cast<uint8_t>(0x01);
    sendCommand(CMD_WIN, QByteArray::fromRawData(reinterpret_cast<const char *>(data), sizeof(data)));
}

void ChessUpBoard::sendAssistance(const QList<AssistanceColour>& colours)
{
    QBitArray bits((2 * colours.size() + 7) / 8 * 8);
    for (int i=0;i<colours.size();++i) {
        int byte = i / 4;
        int bit = byte * 8 + (3-(i % 4)) * 2;
        switch (colours[i]) {
        case Chessboard::AssistanceColour::Red:
            break;
        case Chessboard::AssistanceColour::Blue:
            bits.setBit(bit, 1);
            break;
        case Chessboard::AssistanceColour::Green:
            bits.setBit(bit + 1, 1);
            break;
        }
    }
    QByteArray data;
    data.reserve(1 + colours.size() + 3 / 4);
    data.append(static_cast<uint8_t>(colours.size()));
    data.append(QByteArray::fromRawData(bits.bits(), (bits.size() + 7) / 8));
    sendCommand(CMD_ASSISTANCE, data);
}

}
