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

#include "boardaddress_p.h"
#include "bleconnection.h"
#include "connectionfactory.h"

namespace Chessboard {

Connection *ConnectionFactory::createConnection(const BoardAddress& address, QObject *parent)
{
    assert(address.isValid());
    switch (address.connectionMethod()) {
    case CONNECTION_BLE:
        return new BleConnection(address, address.d_func()->bluetoothDeviceInfo(), parent);
    default:
        return nullptr;
    }
}

}
