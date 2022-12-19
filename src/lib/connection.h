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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>

#include "chessboard.h"

namespace Chessboard {

class Connection : public QObject
{
    Q_OBJECT
public:
    Connection(const BoardAddress& address, QObject *parent = nullptr) :
        QObject(parent), m_address(address) {}
    virtual ~Connection() {};
    virtual void connectToBoard() = 0;
    virtual void disconnectFromBoard() = 0;
    BoardAddress address() const { return m_address; }
signals:
    void connected(Chessboard::RemoteBoard *board);
    void disconnected();
    void error(ConnectionManager::Error error);
    void connectionFailed();
private:
    BoardAddress m_address;
};

}

#endif // CONNECTION_H
