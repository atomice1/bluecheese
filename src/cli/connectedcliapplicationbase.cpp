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

#include "applicationfacade.h"
#include "chessboard.h"
#include "clioptions.h"
#include "connectedcliapplicationbase.h"

using namespace Chessboard;

ConnectedCliApplicationBase::ConnectedCliApplicationBase(const CliOptions &options, QObject *parent)
    : CliApplicationBase{options, parent}
{
    if (!options.address.isValid()) {
        QMetaObject::invokeMethod(facade(), &ApplicationFacade::connectToLast, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(this, [this]() {
                facade()->connectToBoard(this->options<CliOptions>().address);
            }, Qt::QueuedConnection);
    }
}
