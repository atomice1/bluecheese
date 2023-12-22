/*
 * bluecheese
 * Copyright (C) 2023 Chris January
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

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <QObject>
#include "chessboard.h"

class AiPlayer : public QObject {
    Q_OBJECT
public:
    enum Error {
        EngineNotConfigured,
        EngineNotFound,
        EngineIncompatible,
        EngineNoStart,
        EngineNoBoot,
        EngineTimedOut,
        UnknownError
    };

    AiPlayer(Chessboard::Colour colour, QObject *parent);
    virtual ~AiPlayer() {};

    Chessboard::Colour colour() const;
    bool isCancelled() const;
    virtual void cancel();

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw();
    void declineDraw();
    void requestResignation();
    void requestPromotion(Chessboard::Piece piece);
    void error(Error error);

public slots:
    virtual void start(const Chessboard::BoardState& state) = 0;
    virtual void promotionRequired() = 0;
    virtual void drawRequested();
    virtual void drawDeclined();

private:
    Chessboard::Colour m_colour;
    bool m_cancelled {};
};

#endif // AIPLAYER_H
