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

#ifndef CHESSBOARDSCENE_H
#define CHESSBOARDSCENE_H

#include <QFont>
#include <QGraphicsScene>
#include <QMap>
#include <QList>
#include <QPair>
#include "chessboard.h"

class ChessboardScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ChessboardScene(QObject *parent = nullptr);
    Chessboard::BoardState boardState() const;
    Chessboard::Square selectedSquare() const;

signals:
    void squareSelected(int row, int col);
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);

public slots:
    void setBoardState(const Chessboard::BoardState& newState);
    void setGameInProgress(bool flag);
    void setEditMode(bool enabled);
    void setLocalPlayer(Chessboard::Colour colour, bool localPlayer);
    void setAssistance(const QList<Chessboard::AssistanceColour>& colours);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    void update();
    void updateSquares();
    bool isLocalPlayerColour(Chessboard::Colour colour);

    QMap<QPair<Chessboard::Square, Chessboard::ColouredPiece>, QGraphicsItem *> m_itemsBySquare;
    QMap<Chessboard::ColouredPiece, QList<QGraphicsItem *> > m_itemsByPiece;
    QGraphicsRectItem *m_squares[8][8];
    Chessboard::BoardState m_board;
    QFont m_font;
    Chessboard::Square m_from, m_to;
    QMap<Chessboard::Square, QMap<Chessboard::Square, Chessboard::AssistanceColour> > m_assistance;
    bool m_gameInProgress { true };
    bool m_editMode { false };
    bool m_localPlayer[2] {};
};

#endif // CHESSBOARDSCENE_H
