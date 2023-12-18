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

#include "remoteboard_p.h"
#include "chessboard.h"

namespace Chessboard {

RemoteBoardPrivate::RemoteBoardPrivate(const BoardAddress& address) :
    m_address(address)
{
}

RemoteBoard::RemoteBoard(const BoardAddress& address, QObject *parent) :
    QObject(parent),
    d_ptr(new RemoteBoardPrivate(address))
{
}

RemoteBoard::~RemoteBoard()
{
}

BoardAddress RemoteBoard::address() const
{
    Q_D(const RemoteBoard);

    return d->address();
}

void RemoteBoard::requestRemoteBoardState()
{
}

void RemoteBoard::requestMove(int, int, int, int)
{
}

void RemoteBoard::setBoardState(const BoardState&)
{
}

void RemoteBoard::requestNewGame()
{
}

void RemoteBoard::requestNewGame(const Chessboard::GameOptions& gameOptions)
{
}

void RemoteBoard::requestPromotion(Piece)
{
}

void RemoteBoard::requestDraw(Colour requestor)
{
}

void RemoteBoard::requestResignation(Colour requestor)
{
}

}
