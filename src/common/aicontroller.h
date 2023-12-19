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

#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <QObject>
#include "aiplayer.h"
#include "chessboard.h"

class AiPlayer;

class AiController : public QObject
{
    Q_OBJECT
public:
    explicit AiController(QObject *parent = nullptr);
    ~AiController();
    void setAiPlayer(Chessboard::Colour colour, AiPlayer *aiPlayer);

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor);
    void requestPromotion(Chessboard::Piece piece);
    void error(AiPlayer::Error error);

public slots:
    void start(Chessboard::Colour colour, const Chessboard::BoardState& state);
    void cancel();
    void cancel(Chessboard::Colour colour);
    void drawRequested(Chessboard::Colour requestor);
    void promotionRequired(Chessboard::Colour colour);

private:
    AiPlayer **aiPlayerRef(Chessboard::Colour);
    AiPlayer *aiPlayer(Chessboard::Colour);

    QThread *m_thread;
    AiPlayer *m_aiPlayer[2] {};
};

#endif // AICONTROLLER_H
