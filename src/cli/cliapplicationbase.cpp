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
#include <QIODevice>
#include <QTextStream>

#include "applicationfacade.h"
#include "chessboard.h"
#include "cliapplicationbase.h"
#include "clioptions.h"

using namespace Chessboard;

CliApplicationBase::CliApplicationBase(const CliOptions *options, QObject *parent) :
    ApplicationBase(options, parent)
{
    connect(facade(), &ApplicationFacade::connected, this, &CliApplicationBase::onConnected);
    connect(facade(), &ApplicationFacade::disconnected, this, &CliApplicationBase::onDisconnected);
    connect(facade(), &ApplicationFacade::localOutOfSyncWithRemote, this, &CliApplicationBase::onOutOfSync);
    connect(facade(), &ApplicationFacade::noBoardsDiscovered, this, &CliApplicationBase::onNoBoardsDiscovered);
    connect(facade(), &ApplicationFacade::error, this, &CliApplicationBase::onError);
    connect(facade(), &ApplicationFacade::connectionFailed, this, &CliApplicationBase::onConnectionFailed);
    connect(facade(), &ApplicationFacade::noLastConnectedAddress, this, &CliApplicationBase::onNoLastConnectedAddress);
}

bool CliApplicationBase::isQuiet() const
{
    return options<CliOptions>()->quiet;
}

void CliApplicationBase::onConnected(Chessboard::RemoteBoard *board)
{
    if (!isQuiet()) {
        QTextStream ts(stderr, QIODevice::WriteOnly);
        ts << tr("Connected to %1.").arg(board->address().toString()) << "\n";
    }
}

void CliApplicationBase::onDisconnected()
{
    if (!isQuiet()) {
        QTextStream ts(stderr, QIODevice::WriteOnly);
        ts << tr("Disconnected.") << "\n";
    }
    QCoreApplication::exit(1);
}

void CliApplicationBase::onOutOfSync()
{
    QTextStream ts(stderr, QIODevice::WriteOnly);
    ts << tr("Error: out of sync with remote board.") << "\n";
    QCoreApplication::exit(1);
}

void CliApplicationBase::onNoBoardsDiscovered()
{
    QTextStream ts(stderr, QIODevice::WriteOnly);
    ts << tr("No boards discovered.") << "\n";
    QCoreApplication::exit(1);
}

void CliApplicationBase::onError(const QString& errorMessage)
{
    QTextStream ts(stderr, QIODevice::WriteOnly);
    ts << tr("Error: %1").arg(errorMessage) << "\n";
    QCoreApplication::exit(1);
}

void CliApplicationBase::onConnectionFailed()
{
    QTextStream ts(stderr, QIODevice::WriteOnly);
    ts << tr("Connection failed.") << "\n";
    QCoreApplication::exit(1);
}

void CliApplicationBase::onNoLastConnectedAddress()
{
    QTextStream ts(stderr, QIODevice::WriteOnly);
    ts << tr("Specify a board to connect to with --address.") << "\n";
    QCoreApplication::exit(1);
}
