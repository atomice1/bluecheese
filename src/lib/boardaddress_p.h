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

#ifndef BOARDADDRESS_P_H
#define BOARDADDRESS_P_H

#include <QBluetoothDeviceInfo>
#include <QByteArray>
#include <QString>

#include "chessboard.h"

namespace Chessboard {

class BoardAddressPrivate {
public:
    virtual ~BoardAddressPrivate() {}
    virtual bool isValid() const { return false; }
    virtual ConnectionMethod connectionMethod() const = 0;
    virtual QString toString() const = 0;
    virtual QByteArray toByteArray() const = 0;
    virtual QBluetoothDeviceInfo bluetoothDeviceInfo() const { return QBluetoothDeviceInfo(); }
};

class BluetoothBoardAddressPrivate : public BoardAddressPrivate {
public:
    BluetoothBoardAddressPrivate(const QBluetoothDeviceInfo& info);
    BluetoothBoardAddressPrivate(const BluetoothBoardAddressPrivate& other);
    bool isValid() const override { return true; }
    ConnectionMethod connectionMethod() const override { return CONNECTION_BLE; };
    QString toString() const override;
    QByteArray toByteArray() const override;
    QBluetoothDeviceInfo bluetoothDeviceInfo() const override { return info; }
    bool operator==(const BluetoothBoardAddressPrivate& other) const;
    bool operator!=(const BluetoothBoardAddressPrivate& other) const { return !(*this == other); }
    static BluetoothBoardAddressPrivate *fromByteArray(const QByteArray& in);
private:
    QBluetoothDeviceInfo info;
};

class InvalidBoardAddressPrivate : public BoardAddressPrivate {
public:
    ConnectionMethod connectionMethod() const override { return static_cast<ConnectionMethod>(0); }
    QString toString() const override { return QString(); }
    QByteArray toByteArray() const override { return QByteArray(); }
    bool operator==(const InvalidBoardAddressPrivate& other) const { return !other.isValid(); }
    bool operator!=(const InvalidBoardAddressPrivate& other) const { return !(*this == other); }
};

}

#endif // BOARDADDRESS_P_H
