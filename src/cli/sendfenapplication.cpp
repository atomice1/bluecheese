/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include "applicationfacade.h"
#include "sendfenapplication.h"

using namespace Chessboard;

SendFenApplication::SendFenApplication(const QString& address, const QString& fen, QObject *parent)
    : ConnectedCliApplicationBase{address, parent},
      m_fen(BoardState::fromFenString(fen))
{
    connect(facade(), &ApplicationFacade::connected, this, &SendFenApplication::onConnected);
    connect(facade(), &ApplicationFacade::remoteBoardState, this, &SendFenApplication::onRemoteBoardState);
}

void SendFenApplication::onConnected(RemoteBoard *board)
{
    board->setBoardState(m_fen);
}

void SendFenApplication::onRemoteBoardState(const BoardState& newState)
{
    QCoreApplication::exit(0);
}
