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

#include "chessboard.h"
#include "connection.h"
#include "connectionmanager_p.h"

namespace Chessboard {

ConnectionManagerPrivate::ConnectionManagerPrivate(ConnectionManager *q) :
    q_ptr(q)
{
}

ConnectionManagerPrivate::~ConnectionManagerPrivate()
{
    delete m_connection;
}

void ConnectionManagerPrivate::connectToBoard(const BoardAddress& address, QObject *parent)
{
    Q_Q(ConnectionManager);

    delete m_connection;
    assert(m_state == Disconnected);
    m_connection = (m_connectionFactory.createConnection(address, q));
    QObject::connect(m_connection, &Connection::connected, q, [this, parent](RemoteBoard *board){
        Q_Q(ConnectionManager);
        board->setParent(parent);
        m_state = Connected;
        emit q->connected(board);
    });
    QObject::connect(m_connection, &Connection::disconnected, q, [this]() {
        qDebug("ConnectionManager::disconnected");
        Q_Q(ConnectionManager);
        State oldState = m_state;
        m_state = Disconnected;
        if (oldState == Connected)
            emit q->disconnected();
        else if (oldState == Connecting)
            emit q->connectionFailed();
        Connection *oldConnection = m_connection;
        m_connection = nullptr;
        delete m_connection;
    });
    QObject::connect(m_connection, &Connection::error, q, [this](ConnectionManager::Error newError) {
        Q_Q(ConnectionManager);
        emit q->error(newError);
        if (m_state == Connecting) {
            m_state = Disconnected;
            emit q->connectionFailed();
            Connection *oldConnection = m_connection;
            m_connection = nullptr;
            delete m_connection;
        }
    });
    QObject::connect(m_connection, &Connection::connectionFailed, q, [this]() {
        Q_Q(ConnectionManager);
        State oldState = m_state;
        m_state = Disconnected;
        if (oldState == Connecting)
            emit q->connectionFailed();
        Connection *oldConnection = m_connection;
        m_connection = nullptr;
        delete m_connection;
    });
    emit q->connecting(address);
    m_state = Connecting;
    m_connection->connectToBoard();
}

void ConnectionManagerPrivate::disconnectFromBoard()
{
    assert(m_state != Disconnected);
    Q_ASSERT(m_connection != nullptr);
    m_connection->disconnectFromBoard();
}

ConnectionManager::ConnectionManager(QObject *parent) :
    QObject(parent),
    d_ptr(new ConnectionManagerPrivate(this))
{
}

ConnectionManager::~ConnectionManager()
{
}

void ConnectionManager::connectToBoard(const BoardAddress& address, QObject *parent)
{
    Q_D(ConnectionManager);
    d->connectToBoard(address, parent);
}

void ConnectionManager::disconnectFromBoard()
{
    Q_D(ConnectionManager);
    d->disconnectFromBoard();
}

}
