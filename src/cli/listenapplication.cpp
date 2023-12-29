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

#include <QTextStream>

#include "applicationfacade.h"
#include "connectedcliapplicationbase.h"
#include "listenapplication.h"

using namespace Chessboard;

ListenApplication::ListenApplication(const CliOptions& options, QObject *parent)
    : ConnectedCliApplicationBase(options, parent)
{
    connect(facade(), &ApplicationFacade::remoteBoardState, this, &ListenApplication::onRemoteBoardState);
    connect(facade(), &ApplicationFacade::connected, this, [this]() {
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalBoard;
        gameOptions.black.playerType = Chessboard::PlayerType::Human;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalBoard;
        facade()->setGameOptions(gameOptions);
        }, Qt::QueuedConnection);
}

void ListenApplication::onRemoteBoardState(const BoardState& newState)
{
    QTextStream ts(stdout);
    ts << newState.toFenString() << "\n";
}
