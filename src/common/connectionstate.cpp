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
#include "connectionstate.h"

QString connectionStateToString(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Disconnected:
        return QCoreApplication::translate("ConnectionState", "disconnected");
    case ConnectionState::Connecting:
        return QCoreApplication::translate("ConnectionState", "connecting");
    case ConnectionState::Connected:
        return QCoreApplication::translate("ConnectionState", "connected");
    default:
        Q_ASSERT(state == ConnectionState::Disconnected ||
                 state == ConnectionState::Connecting ||
                 state == ConnectionState::Connected);
        return QString();
    }
}

QString connectionStateToStringTitle(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Disconnected:
        return QCoreApplication::translate("ConnectionState", "Disconnected");
    case ConnectionState::Connecting:
        return QCoreApplication::translate("ConnectionState", "Connecting");
    case ConnectionState::Connected:
        return QCoreApplication::translate("ConnectionState", "Connected");
    default:
        Q_ASSERT(state == ConnectionState::Disconnected ||
                 state == ConnectionState::Connecting ||
                 state == ConnectionState::Connected);
        return QString();
    }
}
