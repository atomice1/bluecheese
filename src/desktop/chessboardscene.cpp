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

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QSet>
#include "chessboardscene.h"

namespace {
    qreal SQUARE_SIZE = 10.0;
    qreal BOARD_SIZE = SQUARE_SIZE * 8.0;
    auto FROM_COLOUR = Qt::magenta;
    auto LEGAL_COLOUR = Qt::blue;
    auto TO_COLOUR = Qt::cyan;
    auto CHECK_COLOUR = Qt::red;
    auto BAD_COLOUR = Qt::red;
    auto GOOD_COLOUR = Qt::green;
}

using namespace Chessboard;

ChessboardScene::ChessboardScene(QObject *parent)
    : QGraphicsScene{parent},
      m_font(QApplication::font())
{
    m_font.setPixelSize(SQUARE_SIZE);
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            auto colour = ((col+row)&1) ? Qt::white : Qt::black;
            auto rect = addRect(QRectF(col * SQUARE_SIZE, (7-row) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE),
                                QPen(Qt::NoPen), QBrush(colour));
            rect->setZValue(-1.0);
            m_squares[row][col] = rect;
        }
    }
    update();
}

Chessboard::BoardState ChessboardScene::boardState() const
{
    return m_board;
}

Chessboard::Square ChessboardScene::selectedSquare() const
{
    return m_from;
}

void ChessboardScene::update()
{
    auto oldItemsBySquare = m_itemsBySquare;
    m_itemsBySquare.clear();
    QSet<QGraphicsItem  *> used;
    for (int pass=0;pass<2;++pass) {
        for (int row=0;row<8;++row) {
            for (int col=0;col<8;++col) {
                Chessboard::ColouredPiece piece = m_board[row][col];
                if (piece.isValid()) {
                    QGraphicsItem *item = nullptr;
                    if (pass == 0) {
                        item = oldItemsBySquare.value(qMakePair(Chessboard::Square(row, col), piece));
                        if (!item)
                            continue;
                    }
                    if (!item) {
                        auto items = m_itemsByPiece.value(piece);
                        for (auto x : items) {
                            if (!used.contains(x)) {
                                item = x;
                                break;
                            }
                        }
                    }
                    if (item) {
                        item->show();
                    } else {
                        QChar filledPiece = piece.toUnicode().at(0);
                        if (filledPiece.unicode() < 0x265A)
                            filledPiece = QChar(filledPiece.unicode() + 6);
                        QColor backColour = (piece.colour() == Chessboard::Colour::Black) ? Qt::black : Qt::white;
                        QColor frontColour = (piece.colour() == Chessboard::Colour::Black) ? Qt::white : Qt::black;
                        auto front = addSimpleText(filledPiece, m_font);
                        front->setBrush(backColour);
                        front->setPen(QPen(frontColour, 0.1));
                        front->setZValue(1.0);
                        item = front;
                        m_itemsByPiece[piece].append(item);
                    }
                    QRectF boundingRect = item->boundingRect();
                    item->setPos(col * SQUARE_SIZE + (SQUARE_SIZE - boundingRect.width()) / 2,
                                 (7-row) * SQUARE_SIZE + (SQUARE_SIZE - boundingRect.height()) / 2);
                    used.insert(item);
                    m_itemsBySquare.insert(qMakePair(Chessboard::Square(row, col), piece), item);
                }
            }
        }
    }
    for (auto it = m_itemsByPiece.begin(); it != m_itemsByPiece.end(); ++it) {
        for (auto item : it.value()) {
            if (!used.contains(item))
                item->hide();
        }
    }
}

void ChessboardScene::updateSquares()
{
    bool inCheck = m_board.isCheck();
    ColouredPiece king(m_board.activeColour, Piece::King);
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            Square square(row, col);
            auto baseColour = ((col+row)&1) ? Qt::white : Qt::black;
            auto mixColour = Qt::transparent;
            if (square == m_from) {
                mixColour = FROM_COLOUR;
            } else if (!m_editMode) {
                if (square == m_to) {
                    mixColour = TO_COLOUR;
                } else if (inCheck && m_board[square] == king) {
                    mixColour = CHECK_COLOUR;
                } else if (m_from.isValid() &&
                           !m_to.isValid() &&
                           m_board.isLegalMove(m_from, square)) {
                    Chessboard::AssistanceColour colour = m_assistance[m_from].value(square, Chessboard::AssistanceColour::Blue);
                    switch (colour) {
                    case Chessboard::AssistanceColour::Blue:
                        mixColour = LEGAL_COLOUR;
                        break;
                    case Chessboard::AssistanceColour::Red:
                        mixColour = BAD_COLOUR;
                        break;
                    case Chessboard::AssistanceColour::Green:
                        mixColour = GOOD_COLOUR;
                        break;
                    }
                }
            }
            QColor colour;
            if (mixColour == Qt::transparent) {
                colour = baseColour;
            } else {
                QColor colour1 = baseColour;
                QColor colour2 = mixColour;
                colour = QColor((colour1.red() + colour2.red()) / 2,
                                (colour1.green() + colour2.green()) / 2,
                                (colour1.blue() + colour2.blue()) / 2);
            }
            m_squares[row][col]->setBrush(colour);
        }
    }
}

void ChessboardScene::setBoardState(const Chessboard::BoardState& newState)
{
    m_board = newState;
    m_assistance.clear();
    update();
    updateSquares();
}

void ChessboardScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    const int row = 8 - mouseEvent->scenePos().y() / SQUARE_SIZE;
    const int col = mouseEvent->scenePos().x() / SQUARE_SIZE;
    if (row < 0 || row >= 8 || col < 0 || col >= 8 ||
            mouseEvent->scenePos().x() < 0 ||
            mouseEvent->scenePos().y() < 0) {
        m_from = Square();
        m_to = Square();
        updateSquares();
        if (m_editMode)
            emit squareSelected(-1, -1);
        return;
    }
    Square square(row, col);
    if (mouseEvent->button() == Qt::LeftButton) {
        if ((m_board[square].colour() == m_board.activeColour &&
            isLocalPlayerColour(m_board.activeColour)) ||
            m_editMode) {
            m_from = square;
            m_to = Square();
            updateSquares();
            if (m_editMode)
                emit squareSelected(m_from.row, m_from.col);
        } else if (m_from.isValid() &&
                   m_board.isLegalMove(m_from, square) &&
                   !m_editMode) {
            m_to = square;
            updateSquares();
        }
    } else if (mouseEvent->button() == Qt::RightButton) {
        if (m_to.isValid()) {
            m_to = Square();
            updateSquares();
        } else {
            m_from = Square();
            updateSquares();
            if (m_editMode)
                emit squareSelected(-1, -1);
        }
    }
}

void ChessboardScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton &&
            m_from.isValid() &&
            m_to.isValid() &&
            m_gameInProgress &&
            !m_editMode) {
        emit requestMove(m_from.row, m_from.col, m_to.row, m_to.col);
        m_from = Square();
        m_to = Square();
        updateSquares();
    }
}

bool ChessboardScene::isLocalPlayerColour(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return m_localPlayer[0];
    case Chessboard::Colour::Black:
        return m_localPlayer[1];
    }
}

void ChessboardScene::setGameInProgress(bool flag)
{
    m_gameInProgress = flag;
}

void ChessboardScene::setEditMode(bool enabled)
{
    m_editMode = enabled;
    m_from = Square();
    m_to = Square();
    updateSquares();
    if (enabled)
        emit squareSelected(-1, -1);
}

void ChessboardScene::setLocalPlayer(Chessboard::Colour colour, bool localPlayer)
{
    switch (colour) {
    case Chessboard::Colour::White:
        m_localPlayer[0] = localPlayer;
        break;
    case Chessboard::Colour::Black:
        m_localPlayer[1] = localPlayer;
        break;
    }
}

void ChessboardScene::setAssistance(const QList<Chessboard::AssistanceColour>& colours)
{
    qDebug("ChessboardScene::setAssistance");
    QList<QPair<Square, Square> > legalMoves = m_board.sortedLegalMoves();
    if (legalMoves.size() != colours.size())
        qWarning("Assistance colours array size does not match legal moves array size");
    for (int i=0;i<std::min(legalMoves.size(), colours.size());++i)
        m_assistance[legalMoves[i].first][legalMoves[i].second] = colours[i];
    update();
    updateSquares();
}
