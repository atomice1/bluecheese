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

#ifndef BLEBOARDFACTORY_H
#define BLEBOARDFACTORY_H

class QObject;

namespace Chessboard {

class BleConnection;
class BoardAddress;
class RemoteBoard;

class BleBoardFactory
{
public:
    RemoteBoard *create(const BoardAddress& address, BleConnection *connection, QObject *parent = nullptr);
};

}

#endif // BLEBOARDFACTORY_H
