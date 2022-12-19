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

#ifndef COMPOSITEBOARD_H
#define COMPOSITEBOARD_H

#include "chessboard.h"

class CompositeBoard : public QObject
{
    Q_OBJECT
public:
    explicit CompositeBoard(QObject *parent = nullptr);
    Chessboard::BoardState boardState() const { return m_local; }
public slots:
    void setRemoteBoard(Chessboard::RemoteBoard *board);
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestMove(const Chessboard::Square& from, const Chessboard::Square& to) {
        requestMove(from.row, from.col, to.row, to.col);
    }
    void requestPromotion(Chessboard::Piece piece);
    void setBoardState(const Chessboard::BoardState& boardState);
    void requestNewGame();
    void requestRemoteBoardState();
    void sendLocalBoardState();
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor);
signals:
    void remoteMove(int fromRow, int fromCol, int toRow, int toCol);
    void remotePromotion(Chessboard::Piece piece);
    void remoteBoardState(const Chessboard::BoardState& boardState);
    void promotionRequired();
    void drawRequested(Chessboard::Colour requestor);
    void draw(Chessboard::DrawReason reason);
    void resignation(Chessboard::Colour colour);
    void checkmate(Chessboard::Colour winner);
    void boardStateChanged(const Chessboard::BoardState& boardState);
    void illegalMove(int fromRow, int fromCol, int toRow, int toCol);
    void localOutOfSyncWithRemote();
    void remoteOutOfSyncWithLocal();
private slots:
    void checkGameOver();
private:
    Chessboard::BoardState m_local;
    Chessboard::RemoteBoard *m_remote {};
    bool m_hasLocalMoves {};
    bool m_drawRequested { false };
    Chessboard::Colour m_drawRequestor;
};

#endif // COMPOSITEBOARD_H
