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

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtGlobal>
#include <QDataStream>
#include <QIODevice>
#include <QLowEnergyController>
#include <QSharedData>

#include "boardaddress_p.h"
#include "chessboard.h"

namespace Chessboard {

class BoardDiscoveryPrivate {
public:
    BoardDiscoveryPrivate(BoardDiscovery *q);
    virtual ~BoardDiscoveryPrivate();
private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo& info);
    void errorOccurred(QBluetoothDeviceDiscoveryAgent::Error error);
    void start();
    void stop();
    void onFinished();
private:
    Q_DISABLE_COPY(BoardDiscoveryPrivate)
    Q_DECLARE_PUBLIC(BoardDiscovery)
    BoardDiscovery *q_ptr;
    QScopedPointer<QBluetoothDeviceDiscoveryAgent> m_deviceDiscoveryAgent;
    QList<BoardAddress> m_discoveredBoards;
};

BoardDiscoveryPrivate::BoardDiscoveryPrivate(BoardDiscovery *q) :
    q_ptr(q),
    m_deviceDiscoveryAgent(new QBluetoothDeviceDiscoveryAgent(q_ptr))
{
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(15000);

    QObject::connect(m_deviceDiscoveryAgent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                     q, [this](auto info) { deviceDiscovered(info); });
    QObject::connect(m_deviceDiscoveryAgent.get(), &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                     q, [this](auto error) { errorOccurred(error); });

    QObject::connect(m_deviceDiscoveryAgent.get(), &QBluetoothDeviceDiscoveryAgent::finished,
                     q, [this]() { onFinished(); });
    QObject::connect(m_deviceDiscoveryAgent.get(), &QBluetoothDeviceDiscoveryAgent::canceled,
                     q, &BoardDiscovery::finished);
}

BoardDiscoveryPrivate::~BoardDiscoveryPrivate()
{
}

void BoardDiscoveryPrivate::deviceDiscovered(const QBluetoothDeviceInfo& info)
{
    Q_Q(BoardDiscovery);
    if (!info.name().isEmpty()) {
        BluetoothBoardAddressPrivate *address_d = new BluetoothBoardAddressPrivate(info);
        BoardAddress address(address_d);
        if (!m_discoveredBoards.contains(address)) {
            m_discoveredBoards.append(address);
            emit q->boardDiscovered(address);
        }
    }
}

void BoardDiscoveryPrivate::errorOccurred(QBluetoothDeviceDiscoveryAgent::Error error)
{
    Q_Q(BoardDiscovery);

    BoardDiscovery::Error error2;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        error2 = BoardDiscovery::NoError;
        break;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        error2 = BoardDiscovery::PoweredOffError;
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        error2 = BoardDiscovery::InputOutputError;
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        error2 = BoardDiscovery::InvalidBluetoothAdapterError;
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        error2 = BoardDiscovery::UnsupportedPlatformError;
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        error2 = BoardDiscovery::UnsupportedDiscoveryMethod;
        break;
    case QBluetoothDeviceDiscoveryAgent::LocationServiceTurnedOffError:
        error2 = BoardDiscovery::LocationServiceTurnedOffError;
        break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    case QBluetoothDeviceDiscoveryAgent::MissingPermissionsError:
        error2 = BoardDiscovery::MissingPermissionsError;
        break;
#endif
    case QBluetoothDeviceDiscoveryAgent::UnknownError:
    default:
        error2 = BoardDiscovery::UnknownError;
        break;
    }
    emit q->error(error2);
    onFinished();
}

void BoardDiscoveryPrivate::start()
{
    Q_Q(BoardDiscovery);

    m_discoveredBoards.clear();
    emit q->started();
    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BoardDiscoveryPrivate::stop()
{
    m_deviceDiscoveryAgent->stop();
}

void BoardDiscoveryPrivate::onFinished()
{
    Q_Q(BoardDiscovery);

    emit q->finished();
    if (m_discoveredBoards.isEmpty())
        emit q->noBoardsDiscovered();
    emit q->boardsDiscovered(m_discoveredBoards);
}

BoardDiscovery::BoardDiscovery(QObject *parent) :
    QObject(parent),
    d_ptr(new BoardDiscoveryPrivate(this))
{
}

BoardDiscovery::~BoardDiscovery()
{
}

void BoardDiscovery::start()
{
    Q_D(BoardDiscovery);

    d->start();
}

void BoardDiscovery::stop()
{
    Q_D(BoardDiscovery);

    d->stop();
}

}
