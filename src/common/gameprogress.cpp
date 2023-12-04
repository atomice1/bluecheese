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
#include "commontranslations.h"
#include "gameprogress.h"

using namespace Chessboard;

QString GameProgress::toString() const
{
    switch (state) {
    case GameProgress::InProgress:
        return QCoreApplication::translate("GameProgress", "Game In Progress");
    case GameProgress::Checkmate:
        return QCoreApplication::translate("GameProgress", "Checkmate -- %1 wins").arg(
            colourToString(winner));
    case GameProgress::Resignation:
        return QCoreApplication::translate("GameProgress", "%1 resigns -- %1 wins").arg(
            colourToString(invertColour(winner)),
            colourToString(winner));
    case GameProgress::Draw:
        switch (reason) {
        case DrawReason::DeadPosition:
            return QCoreApplication::translate("GameProgress", "Draw -- Dead Position");
        case DrawReason::Stalemate:
            return QCoreApplication::translate("GameProgress", "Draw -- Stalemate");
        case DrawReason::ThreefoldRepetitionRule:
            return QCoreApplication::translate("GameProgress", "Draw -- Threefold Reptition Rule");
        case DrawReason::FiftyMoveRule:
            return QCoreApplication::translate("GameProgress", "Draw -- 50 Move Rule");
        case DrawReason::FivefoldRepetitionRule:
            return QCoreApplication::translate("GameProgress", "Draw -- Fivefold Reptition Rule");
        case DrawReason::SeventyFiveMoveRule:
            return QCoreApplication::translate("GameProgress", "Draw -- 75 Move Rule");
        case DrawReason::None:
        case DrawReason::MutualAgreement:
            return QCoreApplication::translate("GameProgress", "Draw");
        default:
            Q_ASSERT(reason == DrawReason::DeadPosition ||
                     reason == DrawReason::Stalemate ||
                     reason == DrawReason::ThreefoldRepetitionRule ||
                     reason == DrawReason::FiftyMoveRule ||
                     reason == DrawReason::FivefoldRepetitionRule ||
                     reason == DrawReason::SeventyFiveMoveRule ||
                     reason == DrawReason::None ||
                     reason == DrawReason::MutualAgreement);
            return QString();
        }
    default:
        Q_ASSERT(state == GameProgress::InProgress ||
                 state == GameProgress::Checkmate ||
                 state == GameProgress::Resignation ||
                 state == GameProgress::Draw);
        return QString();
    }
}
