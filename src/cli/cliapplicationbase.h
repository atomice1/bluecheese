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

#ifndef CLIAPPLICATIONBASE_H
#define CLIAPPLICATIONBASE_H

#include "applicationbase.h"
#include "chessboard.h"

class CliOptions;

class CliApplicationBase : public ApplicationBase
{
    Q_OBJECT
public:
    CliApplicationBase(const CliOptions *options, QObject *parent = nullptr);
    bool isQuiet() const;
private slots:
    void onConnected(Chessboard::RemoteBoard *board);
    void onDisconnected();
    void onOutOfSync();
    void onNoBoardsDiscovered();
    void onError(const QString& errorMessage);
    void onConnectionFailed();
    void onNoLastConnectedAddress();
};

#endif // CLIAPPLICATIONBASE_H
