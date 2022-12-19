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

#ifndef BLUETOOTHCONNECTION_H
#define BLUETOOTHCONNECTION_H

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

#include "bleboardfactory.h"
#include "connection.h"

namespace Chessboard {

class BleConnection : public Connection
{
    Q_OBJECT
public:
    BleConnection(const BoardAddress& address, const QBluetoothDeviceInfo& info, QObject *parent = nullptr);
    virtual ~BleConnection();
    void connectToBoard() override;
    void disconnectFromBoard() override;
    QList<QBluetoothUuid> services() const;
    QLowEnergyService *createServiceObject(const QBluetoothUuid &serviceUuid, QObject *parent = nullptr);
    QString name() const { return m_info.name(); }
private slots:
    void controllerErrorOccurred(QLowEnergyController::Error error);
    void serviceErrorOccurred(QLowEnergyService::ServiceError error);
private:
    enum State {
        Disconnected,
        Connecting,
        ConnectedToDeviceDiscoveryInProgress,
        DiscoveryFinished,
        Connected,
        Disconnecting
    };
    QLowEnergyController *m_central;
    Board *m_board;
    QBluetoothDeviceInfo m_info;
    QList<QBluetoothUuid> m_services;
    BleBoardFactory m_factory;
    State m_state { Disconnected };
};

}

#endif // BLUETOOTHCONNECTION_H
