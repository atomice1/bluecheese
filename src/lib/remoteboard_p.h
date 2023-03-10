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

#ifndef BOARD_P_H
#define BOARD_P_H

#include "chessboard.h"

namespace Chessboard {

class RemoteBoardPrivate
{
public:
    RemoteBoardPrivate(const BoardAddress& address);
    BoardAddress address() const { return m_address; }
private:
    BoardAddress m_address;
};

}

#endif // BOARD_P_H
