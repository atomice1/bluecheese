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

#ifndef CONNECTIONMANAGER_P_H
#define CONNECTIONMANAGER_P_H

#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QByteArray>
#include <QLatin1String>

#include "chessboard.h"
#include "connectionfactory.h"

namespace Chessboard {

class Connection;
class ConnectionManagerPrivate;

class ConnectionManagerPrivate {
public:
    ConnectionManagerPrivate(ConnectionManager *q);
    virtual ~ConnectionManagerPrivate();
    void connectToBoard(const BoardAddress& address, QObject *parent = nullptr);
    void disconnectFromBoard();
private:
    enum State {
        Disconnected,
        Connecting,
        Connected
    };

    Q_DISABLE_COPY(ConnectionManagerPrivate)
    Q_DECLARE_PUBLIC(ConnectionManager)
    ConnectionManager *q_ptr;
    ConnectionFactory m_connectionFactory;
    Connection *m_connection {};
    State m_state {Disconnected};
};

}

#endif // CONNECTIONMANAGER_P_H
