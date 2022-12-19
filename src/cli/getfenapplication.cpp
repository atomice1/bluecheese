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
#include "getfenapplication.h"

using namespace Chessboard;

GetFenApplication::GetFenApplication(const QString& address, QObject *parent)
    : ConnectedCliApplicationBase{address, parent}
{
    connect(facade(), &ApplicationFacade::remoteBoardState, this, &GetFenApplication::onRemoteBoardState);
}

void GetFenApplication::onRemoteBoardState(const BoardState& newState)
{
    QTextStream ts(stdout);
    ts << newState.toFenString() << "\n";
    QCoreApplication::exit(0);
}
