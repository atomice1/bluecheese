/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "aicontroller.h"
#include "applicationfacade.h"
#include "commontranslations.h"
#include "compositeboard.h"

using namespace Chessboard;

namespace {
    const QLatin1String ORGANIZATION_NAME("atomice.com");
    const QLatin1String APPLICATION_NAME("chessboard");
    const QLatin1String CONNECTION_GROUP("connection");
    const QLatin1String ADDRESS("address");
}

ApplicationFacade::ApplicationFacade(QObject *parent)
    : QObject{parent}
{
    m_boardDiscovery = new BoardDiscovery(this);
    connect(m_boardDiscovery, &BoardDiscovery::finished, this, &ApplicationFacade::discoveryFinished);
    connect(m_boardDiscovery, &BoardDiscovery::error, this, &ApplicationFacade::onDiscoveryError);
    connect(m_boardDiscovery, &BoardDiscovery::boardDiscovered, this, &ApplicationFacade::boardDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::boardsDiscovered, this, &ApplicationFacade::boardsDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::noBoardsDiscovered, this, &ApplicationFacade::noBoardsDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::started, this, &ApplicationFacade::discoveryStarted);

    m_connectionManager = new ConnectionManager(this);
    connect(m_connectionManager, &ConnectionManager::connected, this, [this](RemoteBoard *board) {
        m_board->setRemoteBoard(board);
        emit connected(board);
    });
    connect(m_connectionManager, &ConnectionManager::disconnected, this, [this]() {
        qDebug("ApplicationFacade::disconnected");
        emit disconnected();
        m_board->setRemoteBoard(nullptr);
    });
    connect(m_connectionManager, &ConnectionManager::error, this, &ApplicationFacade::onConnectionError);
    connect(m_connectionManager, &ConnectionManager::connectionFailed, this, &ApplicationFacade::connectionFailed);
    connect(m_connectionManager, &ConnectionManager::connecting, this, &ApplicationFacade::connecting);

    m_board = new CompositeBoard(this);
    connect(m_board, &CompositeBoard::remoteMove, this, &ApplicationFacade::remoteMove);
    connect(m_board, &CompositeBoard::remoteBoardState, this, &ApplicationFacade::remoteBoardState);
    connect(m_board, &CompositeBoard::localOutOfSyncWithRemote, this, &ApplicationFacade::localOutOfSyncWithRemote);
    connect(m_board, &CompositeBoard::illegalMove, this, &ApplicationFacade::illegalMove);
    connect(m_board, &CompositeBoard::remoteOutOfSyncWithLocal, this, &ApplicationFacade::remoteOutOfSyncWithLocal);
    connect(m_board, &CompositeBoard::boardStateChanged, this, [this](const BoardState& state) {
        m_aiController->cancel();
        emit boardStateChanged(state);
        emit gameProgressChanged(GameProgress(GameProgress::InProgress));
        emit activeColourChanged(state.activeColour);
    });
    connect(m_board, &CompositeBoard::promotionRequired, [this]() {
        if (!isCurrentPlayerAppAi())
            emit promotionRequired();
    });
    connect(m_board, &CompositeBoard::drawRequested, [this](Colour requestor) {
        if (!isPlayerAppAi(invertColour(requestor)))
            emit drawRequested(requestor);
    });
    connect(m_board, &CompositeBoard::resignation, this, [this](Colour colour) {
        emit resignation(colour);
        emit gameOver();
        emit gameProgressChanged(GameProgress(GameProgress::Resignation,
                                              (colour == Colour::White) ? Colour::Black : Colour::White));
    });
    connect(m_board, &CompositeBoard::checkmate, this, [this](Colour colour) {
        emit checkmate(colour);
        emit gameOver();
        emit gameProgressChanged(GameProgress(GameProgress::Checkmate, colour));
    });
    connect(m_board, &CompositeBoard::draw, this, [this](DrawReason reason) {
        emit draw(reason);
        emit gameOver();
        emit gameProgressChanged(GameProgress(GameProgress::Draw, reason));
    });

    m_aiController = new AiController(this);
    connect(this, &ApplicationFacade::activeColourChanged, [this](Colour) {
        if (isCurrentPlayerAppAi())
            m_aiController->start(m_board->boardState());
        else
            m_aiController->cancel();
    });
    connect(this, &ApplicationFacade::promotionRequired, [this]() {
        if (isCurrentPlayerAppAi())
            m_aiController->promotionRequired();
    });
    connect(this, &ApplicationFacade::drawRequested, [this](Colour requestor) {
        if (isPlayerAppAi(invertColour(requestor)))
            m_aiController->drawRequested(requestor);
    });
    connect(this, &ApplicationFacade::gameOver, [this]() {
       m_aiController->cancel();
    });

    m_settings.beginGroup(CONNECTION_GROUP);
    m_lastConnectedAddress = BoardAddress::fromByteArray(m_settings.value(ADDRESS, QByteArray()).toByteArray());
    m_settings.endGroup();
}

void ApplicationFacade::connectToLast()
{
    qDebug("ApplicationFacade::connectToLast()");
    if (m_lastConnectedAddress.isValid())
        m_connectionManager->connectToBoard(m_lastConnectedAddress, m_board);
    else
        emit noLastConnectedAddress();
}

void ApplicationFacade::connectToLastOrDiscover()
{
    qDebug("ApplicationFacade::connectToLastOrDiscovery()");
    if (m_lastConnectedAddress.isValid())
        m_connectionManager->connectToBoard(m_lastConnectedAddress, m_board);
    else
        m_boardDiscovery->start();
}

void ApplicationFacade::connectToBoard(const BoardAddress& address)
{
    qDebug("ApplicationFacade::connectToBoard(%s)", qPrintable(address.toString()));
    setLastConnectedAddress(address);
    m_connectionManager->connectToBoard(address, this);
}

void ApplicationFacade::startDiscovery()
{
    qDebug("ApplicationFacade::startDiscovery");
    m_boardDiscovery->start();
}

void ApplicationFacade::stopDiscovery()
{
    qDebug("ApplicationFacade::stopDiscovery");
    m_boardDiscovery->stop();
}

void ApplicationFacade::disconnectFromBoard()
{
    qDebug("ApplicationFacade::disconnectFromBoard");
    m_connectionManager->disconnectFromBoard();
}

void ApplicationFacade::onDiscoveryError(BoardDiscovery::Error error)
{
    qDebug("ApplicationFacade::discoveryError(%d)", error);
    QString errorMessage;
    switch (error) {
    case BoardDiscovery::PoweredOffError:
        errorMessage = tr("Bluetooth powered off / radio disabled");
        break;
    case BoardDiscovery::InputOutputError:
        errorMessage = tr("input/output error");
        break;
    case BoardDiscovery::InvalidBluetoothAdapterError:
        errorMessage = tr("invalid Bluetooth adapter");
        break;
    case BoardDiscovery::UnsupportedPlatformError:
        errorMessage = tr("unsupported platform");
        break;
    case BoardDiscovery::UnsupportedDiscoveryMethod:
        errorMessage = tr("unsupported discovery method");
        break;
    case BoardDiscovery::LocationServiceTurnedOffError:
        errorMessage = tr("Location Services turned off / disabled");
        break;
    case BoardDiscovery::MissingPermissionsError:
        errorMessage = tr("missing permissions");
        break;
    default:
        errorMessage = tr("unknown error");
        break;
    }
    emit this->error(errorMessage);
}

void ApplicationFacade::onConnectionError(ConnectionManager::Error error)
{
    qDebug("ApplicationFacade::onConnectionError(%d)", error);
    QString errorMessage;
    switch (error) {
    case ConnectionManager::UnknownRemoteDeviceError:
        errorMessage = tr("unknown remote device");
        break;
    case ConnectionManager::NetworkError:
        errorMessage = tr("network error");
        break;
    case ConnectionManager::InvalidBluetoothAdapterError:
        errorMessage = tr("invalid Bluetooth adapter");
        break;
    case ConnectionManager::ConnectionError:
        errorMessage = tr("connection error");
        break;
    case ConnectionManager::AdvertisingError:
        errorMessage = tr("advertising error");
        break;
    case ConnectionManager::RemoteHostClosedError:
        errorMessage = tr("remote host closed");
        break;
    case ConnectionManager::AuthorizationError:
        errorMessage = tr("authorization error");
        break;
    case ConnectionManager::MissingPermissionsError:
        errorMessage = tr("missing permissions");
        break;
    case ConnectionManager::DisconnectDuringServiceDiscovery:
        errorMessage = tr("disconnected during service discovery");
        break;
    case ConnectionManager::DisconnectDuringCharacteristicDiscovery:
        errorMessage = tr("disconnected during characteristic discovery");
        break;
    case ConnectionManager::NotSupported:
        errorMessage = tr("device is not supported");
        break;
    default:
        errorMessage = tr("unknown error");
        break;
    }
    emit this->error(errorMessage);
}

void ApplicationFacade::setLastConnectedAddress(const Chessboard::BoardAddress& address)
{
    qDebug("ApplicationFacade::setLastConnectedAddress(%s)", qPrintable(address.toString()));
    m_lastConnectedAddress = address;
    m_settings.beginGroup(CONNECTION_GROUP);
    m_settings.setValue(ADDRESS, m_lastConnectedAddress.toByteArray());
    m_settings.endGroup();
}

void ApplicationFacade::setBoardStateFromFen(const QString& fen)
{
    qDebug("ApplicationFacade::setBoardStateFromFen(%s)", qPrintable(fen));
    BoardState state = BoardState::fromFenString(fen);
    if (!state.isValid()) {
        emit invalidFenString(fen);
    } else {
        if (m_board)
            m_board->setBoardState(state);
    }
}

void ApplicationFacade::requestRemoteBoardState()
{
    qDebug("ApplicationFacade::requestRemoteBoardState()");
    m_board->requestRemoteBoardState();
}

void ApplicationFacade::sendLocalBoardState()
{
    qDebug("ApplicationFacade::sendLocalBoardState()");
    m_board->sendLocalBoardState();
}

void ApplicationFacade::requestNewGame()
{
    qDebug("ApplicationFacade::requestNewGame()");
    m_board->requestNewGame();
}

void ApplicationFacade::requestNewGameOptions(const Chessboard::GameOptions& gameOptions)
{
    qDebug("ApplicationFacade::requestNewGameOptions(...)");
    m_gameOptions = gameOptions;
    m_board->requestNewGameOptions(gameOptions);
}

void ApplicationFacade::requestMove(int fromRow, int fromCol, int toRow, int toCol)
{
    qDebug("ApplicationFacade::requestMove(%d, %d, %d, %d)", fromRow, fromCol, toRow, toCol);
    m_board->requestMove(fromRow, fromCol, toRow, toCol);
}

void ApplicationFacade::requestDraw(Chessboard::Colour requestor)
{
    qDebug("ApplicationFacade::requestDraw(%s)", (requestor == Colour::White) ? "white" : "black");
    m_board->requestDraw(requestor);
}

void ApplicationFacade::requestResignation(Chessboard::Colour requestor)
{
    qDebug("ApplicationFacade::requestResignation(%s)", (requestor == Colour::White) ? "white" : "black");
    m_board->requestResignation(requestor);
}

void ApplicationFacade::requestPromotion(Chessboard::Piece piece)
{
    qDebug("ApplicationFacade::requestPromotion(%s)",
           qPrintable(pieceToString(piece)));
    m_board->requestPromotion(piece);
}

bool ApplicationFacade::isCurrentPlayerAppAi() const
{
    return isPlayerAppAi(m_board->boardState().activeColour);
}

bool ApplicationFacade::isPlayerAppAi(Colour colour) const
{
    const PlayerOptions& playerOptions =
        colour == Colour::Black ? m_gameOptions.black : m_gameOptions.white;
    return playerOptions.playerType == PlayerType::Ai &&
           playerOptions.playerLocation == PlayerLocation::LocalApp;
}
