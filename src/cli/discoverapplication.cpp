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
#include <QTextStream>
#include "applicationfacade.h"
#include "discoverapplication.h"

DiscoverApplication::DiscoverApplication(QObject *parent)
    : CliApplicationBase{parent}
{
    connect(facade(), &ApplicationFacade::boardDiscovered, this, &DiscoverApplication::onBoardDiscovered);
    connect(facade(), &ApplicationFacade::discoveryFinished, this, &DiscoverApplication::onDiscoveryFinished);
    QMetaObject::invokeMethod(facade(), &ApplicationFacade::startDiscovery, Qt::QueuedConnection);
}

void DiscoverApplication::onBoardDiscovered(const Chessboard::BoardAddress& address)
{
    QTextStream ts(stdout);
    ts << address.toString() << "\n";
}

void DiscoverApplication::onDiscoveryFinished()
{
    // Queue the exit so we have time to print "No board discovered." if necessary.
    QMetaObject::invokeMethod(this, [this]() {
        QCoreApplication::exit(0);
    }, Qt::QueuedConnection);
}
