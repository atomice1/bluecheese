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

#ifndef CHESSUPBOARD_H
#define CHESSUPBOARD_H

#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include "chessboard.h"

class QLowEnergyService;

namespace Chessboard {

class BleConnection;

const QLatin1String CHESSUP_SERVICE("{6E400001-B5A3-F393-E0A9-E50E24DCCA9E}"); // Nordic UART Service

class ChessUpBoard : public RemoteBoard
{
    Q_OBJECT
public:
    ChessUpBoard(const BoardAddress& address, BleConnection *connection, QObject *parent = nullptr);
public slots:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol) override;
    void requestRemoteBoardState() override;
    void requestNewGame() override;
    void requestPromotion(Piece piece) override;
    // TODO
    //void requestDraw(Colour requestor);
    //void requestResignation(Colour requestor);
    void setBoardState(const Chessboard::BoardState& boardState) override;
private slots:
    void readFromBoard(const QByteArray& data);
    void writeToBoard(const QByteArray& data);
    void sendCommand(uint8_t cmd, const QByteArray& payload = QByteArray());
    void sendInit();
    void discoveryFinished();
private:
    BoardState boardStateFromRemote(const QByteArray& data);

    BleConnection *m_connection;
    QLowEnergyService *m_uartService;
    QLowEnergyService *m_batteryService;
    QLowEnergyCharacteristic m_rxCharacteristic;
    QLowEnergyCharacteristic m_txCharacteristic;
    QLowEnergyCharacteristic m_batteryCharacteristic;
    QLowEnergyDescriptor m_rxClientCharacteristicConfiguration;
    QLowEnergyDescriptor m_batteryClientCharacteristicConfiguration;
    QByteArray m_previousMove;
    bool m_writeAcked;
    bool m_responseRead;
};

}

#endif // CHESSUPBOARD_H
