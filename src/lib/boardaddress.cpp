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

#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QDataStream>
#include <QIODevice>

#include "boardaddress_p.h"
#include "chessboard.h"

namespace Chessboard {

BoardAddress::BoardAddress() :
    d_ptr(new InvalidBoardAddressPrivate)
{
}

BoardAddress::BoardAddress(const BoardAddress& other) :
    d_ptr(nullptr)
{
    *this = other;
}

BoardAddress::BoardAddress(const BoardAddressPrivate *d) :
    d_ptr(d)
{
}

BoardAddress::~BoardAddress()
{
}

bool BoardAddress::isValid() const
{
    Q_D(const BoardAddress);
    return d->isValid();
}

ConnectionMethod BoardAddress::connectionMethod() const
{
    Q_D(const BoardAddress);
    return d->connectionMethod();
}

QString BoardAddress::toString() const
{
    Q_D(const BoardAddress);
    return d->toString();
}

QByteArray BoardAddress::toByteArray() const
{
    Q_D(const BoardAddress);
    return d->toByteArray();
}

BoardAddress BoardAddress::fromByteArray(const QByteArray& in)
{
    QDataStream ds(in);
    ConnectionMethod method;
    ds >> method;
    if (method == CONNECTION_BLE) {
        return BoardAddress(BluetoothBoardAddressPrivate::fromByteArray(in));
    } else {
        return BoardAddress();
    }
}

BoardAddress& BoardAddress::operator=(const BoardAddress& other)
{
    if (this == &other)
        return *this;
    if (!other.isValid()) {
        d_ptr.reset(new InvalidBoardAddressPrivate);
    } else {
        switch (other.connectionMethod()) {
        case CONNECTION_BLE:
            d_ptr.reset(new BluetoothBoardAddressPrivate(static_cast<const BluetoothBoardAddressPrivate&>(*other.d_func())));
            break;
        }
    }
    return *this;
}

bool BoardAddress::operator==(const BoardAddress& other) const
{
    Q_D(const BoardAddress);
    if (!isValid()) {
        return !other.isValid();
    } else {
        switch (other.connectionMethod()) {
        case CONNECTION_BLE:
            return static_cast<const BluetoothBoardAddressPrivate&>(*d) == static_cast<const BluetoothBoardAddressPrivate&>(*other.d_func());
        default:
            Q_ASSERT(other.connectionMethod() == CONNECTION_BLE);
            return false;
        }
    }
}

bool BoardAddress::operator!=(const BoardAddress& other) const
{
    return !(*this == other);
}

BoardAddress BoardAddress::fromString(const QString& s)
{
    if (s.startsWith("{")) {
        QBluetoothUuid uuid(s);
        if (uuid.isNull())
            return BoardAddress();
        // TODO: The address / UUID alone are insufficient for detection
        // since multiple boards may have the same services and be distinguishable
        // only by name.
        // The only way to get the name of a peripheral form the address / UUID appears
        // to be to enter discovery.
        QBluetoothDeviceInfo info(uuid, "ChessUp", 0);
        return BoardAddress(new BluetoothBoardAddressPrivate(info));
    } else if (s.contains(":")) {
        QBluetoothAddress address(s);
        if (address.isNull())
            return BoardAddress();
        QBluetoothDeviceInfo info(address, "ChessUp", 0);
        return BoardAddress(new BluetoothBoardAddressPrivate(info));
    } else {
        return BoardAddress();
    }
}

BluetoothBoardAddressPrivate::BluetoothBoardAddressPrivate(const QBluetoothDeviceInfo& info_) :
    info(info_)
{
}

BluetoothBoardAddressPrivate::BluetoothBoardAddressPrivate(const BluetoothBoardAddressPrivate& other) :
    info(other.info)
{
}

QString BluetoothBoardAddressPrivate::toString() const
{
    if (info.deviceUuid() != QBluetoothUuid())
        return QString::fromLatin1("%1 %2").arg(info.name(), info.deviceUuid().toString());
    else
        return QString::fromLatin1("%1 %2)").arg(info.name(), info.address().toString());
}

QByteArray BluetoothBoardAddressPrivate::toByteArray() const
{
    QByteArray out;
    {
        QDataStream ds(&out, QIODevice::WriteOnly);
        ds << CONNECTION_BLE;
        if (info.deviceUuid() != QBluetoothUuid()) {
            ds << true;
            ds << info.deviceUuid();
        } else {
            ds << false;
            ds << info.address().toUInt64();
        }
        ds << info.name();
        quint32 classOfDevice = 0;
        classOfDevice |= info.minorDeviceClass() << 2;
        classOfDevice |= info.majorDeviceClass() << 8;
        classOfDevice |= info.serviceClasses() << 13;
        ds << classOfDevice;
    }
    return out;
}

bool BluetoothBoardAddressPrivate::operator==(const BluetoothBoardAddressPrivate& other) const
{
    if (info.deviceUuid() != QBluetoothUuid())
        return info.deviceUuid() == other.info.deviceUuid();
    else
        return info.address() == other.info.address();
}

BluetoothBoardAddressPrivate *BluetoothBoardAddressPrivate::fromByteArray(const QByteArray& in)
{
    QDataStream ds(in);
    ConnectionMethod method;
    ds >> method;
    assert(method == CONNECTION_BLE);
    bool useUuid = false;
    ds >> useUuid;
    if (useUuid) {
        QBluetoothUuid deviceUuid;
        ds >> deviceUuid;
        QString name;
        ds >> name;
        quint32 classOfDevice;
        ds >> classOfDevice;
        return new BluetoothBoardAddressPrivate(QBluetoothDeviceInfo(deviceUuid, name, classOfDevice));
    } else {
        quint64 address;
        ds >> address;
        QString name;
        ds >> name;
        quint32 classOfDevice;
        ds >> classOfDevice;
        return new BluetoothBoardAddressPrivate(QBluetoothDeviceInfo(QBluetoothAddress(address), name, classOfDevice));
    }
}

}
