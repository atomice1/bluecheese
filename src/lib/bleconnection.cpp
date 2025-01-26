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
#include <QBluetoothUuid>
#include <QLatin1String>
#include <QLowEnergyController>
#include <QtGlobal>
#include <QTimer>

#include "bleconnection.h"
#include "chessboard.h"

namespace Chessboard {

BleConnection::BleConnection(const BoardAddress& address, const QBluetoothDeviceInfo& info, QObject *parent) :
    Connection(address, parent),
    m_info(info)
{
    m_central = QLowEnergyController::createCentral(info, this);
    connect(m_central, &QLowEnergyController::serviceDiscovered, this, [this](const QBluetoothUuid& newService) {
        m_services.append(newService);
    });
    connect(m_central, &QLowEnergyController::discoveryFinished, this, [this]() {
        m_state = DiscoveryFinished;
        RemoteBoard *board = m_factory.create(this->address(), this, this);
        if (board) {
            connect(this, &BleConnection::connectionFailed, board, &RemoteBoard::deleteLater);
        } else {
            emit error(ConnectionManager::NotSupported);
            disconnectFromBoard();
        }
    });
    connect(m_central, &QLowEnergyController::connected, this, [this]() {
        qDebug("BleConnection::connected");
        m_state = ConnectedToDeviceDiscoveryInProgress;
        m_central->discoverServices();
    });
    connect(m_central, &QLowEnergyController::disconnected, this, [this]() {
        qDebug("BleConnection::disconnected");
        State oldState = m_state;
        m_state = Disconnected;
        switch (oldState) {
        case Connected:
        case Disconnecting:
            emit disconnected();
            break;
        case DiscoveryFinished:
            emit error(ConnectionManager::DisconnectDuringCharacteristicDiscovery);
            emit connectionFailed();
            break;
        case ConnectedToDeviceDiscoveryInProgress:
            emit error(ConnectionManager::DisconnectDuringServiceDiscovery);
            emit connectionFailed();
            break;
        default:
            break;
        }
    });
    connect(m_central, &QLowEnergyController::errorOccurred, this, &BleConnection::controllerErrorOccurred);
    connect(this, &BleConnection::connected, this, [this]() {
        m_state = Connected;
    });
}

BleConnection::~BleConnection()
{
}

void BleConnection::startDiscovery()
{
    qDebug("BleConnection::startDiscovery");
    m_discoveryStarted = true;
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent;

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, [this](const QBluetoothDeviceInfo& info) {
        qDebug("BleConnection::deviceDiscovered");
        if (info.address() == m_info.address()) {
            m_deviceDiscovered = true;
            m_discoveryAgent->stop();
            QMetaObject::invokeMethod(this, [this]() {
                m_central->connectToDevice();
            }, Qt::QueuedConnection);
        }
    });
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, [this]() {
        qDebug("BleConnection::discoveryFinished");
        if (!m_deviceDiscovered)
            controllerErrorOccurred(QLowEnergyController::UnknownRemoteDeviceError);
    });
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, [this]() {
        qDebug("BleConnection::discoveryErrorOccurred");
        if (!m_deviceDiscovered)
            controllerErrorOccurred(QLowEnergyController::UnknownRemoteDeviceError);
    });
    m_discoveryAgent->start();
}

void BleConnection::controllerErrorOccurred(QLowEnergyController::Error error)
{
    qDebug("BleConnection::controllerErrorOccurred(%d)", error);
    if (error == QLowEnergyController::UnknownRemoteDeviceError && !m_discoveryStarted) {
        // With the bluez stack we may need to re-discover an existing address before we can connect.
        QMetaObject::invokeMethod(this, [this]() {
            startDiscovery();
        }, Qt::QueuedConnection);
        return;
    }
#ifdef __linux__
    if (error == QLowEnergyController::UnknownError && m_connectRetries++ < 5) {
        QTimer::singleShot(250, this, [this]() {
            m_central->connectToDevice();
        });
        return;
    }
#endif
    ConnectionManager::Error error2 = ConnectionManager::UnknownError;
    switch (error) {
    case QLowEnergyController::NoError:
        error2 = ConnectionManager::NoError;
        break;
    case QLowEnergyController::UnknownRemoteDeviceError:
        error2 = ConnectionManager::UnknownRemoteDeviceError;
        break;
    case QLowEnergyController::NetworkError:
        error2 = ConnectionManager::NetworkError;
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        error2 = ConnectionManager::InvalidBluetoothAdapterError;
        break;
    case QLowEnergyController::ConnectionError:
        error2 = ConnectionManager::ConnectionError;
        break;
    case QLowEnergyController::AdvertisingError:
        error2 = ConnectionManager::AdvertisingError;
        break;
    case QLowEnergyController::RemoteHostClosedError:
        error2 = ConnectionManager::RemoteHostClosedError;
        break;
    case QLowEnergyController::AuthorizationError:
        error2 = ConnectionManager::AuthorizationError;
        break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    case QLowEnergyController::MissingPermissionsError:
        error2 = ConnectionManager::MissingPermissionsError;
        break;
#endif
    case QLowEnergyController::UnknownError:
    default:
        error2 = ConnectionManager::UnknownError;
        break;
    }
    emit this->error(error2);
    if (m_state != Connected) {
        m_state = Connected;
        emit connectionFailed();
    }
}

void BleConnection::serviceErrorOccurred(QLowEnergyService::ServiceError error)
{
    qDebug("BleConnection::serviceErrorOccurred(%d)", error);
    ConnectionManager::Error error2 = ConnectionManager::UnknownError;
    switch (error) {
    case QLowEnergyService::NoError:
        error2 = ConnectionManager::NoError;
        break;
    case QLowEnergyService::OperationError:
        error2 = ConnectionManager::OperationError;
        break;
    case QLowEnergyService::CharacteristicWriteError:
        error2 = ConnectionManager::CharacteristicWriteError;
        break;
    case QLowEnergyService::DescriptorWriteError:
        error2 = ConnectionManager::DescriptorWriteError;
        break;
    case QLowEnergyService::CharacteristicReadError:
        error2 = ConnectionManager::CharacteristicReadError;
        break;
    case QLowEnergyService::DescriptorReadError:
        error2 = ConnectionManager::DescriptorReadError;
        break;
    case QLowEnergyService::UnknownError:
    default:
        error2 = ConnectionManager::UnknownError;
        break;
    }
    emit this->error(error2);
    if (m_state != Connected) {
        m_state = Connected;
        emit connectionFailed();
    }
}

void BleConnection::connectToBoard()
{
    qDebug("BleConnection::connectToBoard");
    Q_ASSERT(m_state == Disconnected);
    m_state = Connecting;
    m_central->connectToDevice();
}

void BleConnection::disconnectFromBoard()
{
    qDebug("BleConnection::disconnectFromDevice");
    if (m_state != Disconnected) {
        if (m_state == Connecting) {
            m_state = Disconnected;
            emit disconnected();
        } else {
            m_state = Disconnecting;
            m_central->disconnectFromDevice();
        }
    }
}

QList<QBluetoothUuid> BleConnection::services() const
{
    return m_services;
}

QLowEnergyService *BleConnection::createServiceObject(const QBluetoothUuid &serviceUuid, QObject *parent)
{
    std::unique_ptr<QLowEnergyService> service(m_central->createServiceObject(serviceUuid, parent));
    connect(service.get(), &QLowEnergyService::errorOccurred, this, &BleConnection::serviceErrorOccurred);
    return service.release();
}

}
