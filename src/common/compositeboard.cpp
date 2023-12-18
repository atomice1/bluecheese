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

#include "compositeboard.h"

using namespace Chessboard;

CompositeBoard::CompositeBoard(QObject *parent)
    : QObject{parent},
      m_local(BoardState::newGame())
{
}

void CompositeBoard::setRemoteBoard(Chessboard::RemoteBoard *board)
{
    if (m_remote) {
        disconnect(m_remote, &RemoteBoard::remoteMove, this, nullptr);
        disconnect(m_remote, &RemoteBoard::remoteBoardState, this, nullptr);
        disconnect(m_remote, &RemoteBoard::remotePromotion, this, nullptr);
        disconnect(m_remote, &RemoteBoard::remoteDraw, this, nullptr);
        disconnect(m_remote, &RemoteBoard::remoteCheckmate, this, nullptr);
        disconnect(m_remote, &RemoteBoard::remoteResignation, this, nullptr);
    }
    auto oldBoard = m_remote;
    m_remote = board;
    if (oldBoard != board)
        delete oldBoard;
    if (board) {
        board->setParent(this);
        connect(board, &RemoteBoard::remoteMove, this, [this](int fromRow, int fromCol, int toRow, int toCol) {
            bool legal = true;
            if (!m_hasLocalMoves)
                legal = m_local.move(fromRow, fromCol, toRow, toCol);
            emit remoteMove(fromRow, fromCol, toRow, toCol);
            if (!m_hasLocalMoves && legal)
                emit boardStateChanged(m_local);
            else if (!legal)
                emit localOutOfSyncWithRemote();
            checkGameOver();
        });
        connect(board, &RemoteBoard::remoteBoardState, this, [this](const BoardState& state) {
            if (!m_hasLocalMoves)
                m_local = state;
            emit remoteBoardState(state);
            if (!m_hasLocalMoves)
                emit boardStateChanged(state);
        });
        connect(board, &RemoteBoard::remotePromotion, this, [this](Piece piece) {
            if (!m_hasLocalMoves) {
                bool ok = m_local.promote(piece);
                emit remotePromotion(piece);
                if (ok)
                    emit boardStateChanged(m_local);
                else
                    emit localOutOfSyncWithRemote();
            }
        });
        connect(board, &RemoteBoard::remoteDraw, this, &CompositeBoard::draw);
        connect(board, &RemoteBoard::remoteCheckmate, this, &CompositeBoard::checkmate);
        connect(board, &RemoteBoard::remoteResignation, this, &CompositeBoard::resignation);
        connect(board, &RemoteBoard::remoteDrawRequested, this, &CompositeBoard::drawRequested);
        if (m_hasLocalMoves)
            emit remoteOutOfSyncWithLocal();
    }
}

void CompositeBoard::requestMove(int fromRow, int fromCol, int toRow, int toCol)
{
    qDebug("CompositeBoard::requestMove(%d, %d, %d, %d)", fromRow, fromCol, toRow, toCol);
    m_drawRequested = false;
    bool promotion = false;
    bool ok = m_local.move(fromRow, fromCol, toRow, toCol, &promotion);
    if (!ok)
        emit illegalMove(fromRow, fromCol, toRow, toCol);
    if (m_remote) {
        m_remote->requestMove(fromRow, fromCol, toRow, toCol);
    } else {
        m_hasLocalMoves = true;
        if (promotion)
            emit promotionRequired();
    }
    if (ok)
        emit boardStateChanged(m_local);
    checkGameOver();
}

void CompositeBoard::checkGameOver()
{
    DrawReason reason = DrawReason::None;
    if (m_local.isCheckmate()) {
        m_drawRequested = false;
        emit checkmate((m_local.activeColour == Colour::White) ? Colour::Black : Colour::White);
    } else if (m_local.isAutomaticDraw(&reason)) {
        m_drawRequested = false;
        emit draw(reason);
    }
}

void CompositeBoard::setBoardState(const Chessboard::BoardState& boardState)
{
    m_drawRequested = false;
    m_local = boardState;
    if (m_remote)
        m_remote->setBoardState(boardState);
    else
        m_hasLocalMoves = true;
    emit boardStateChanged(m_local);
    checkGameOver();
}

void CompositeBoard::requestRemoteBoardState()
{
    if (m_remote) {
        m_hasLocalMoves = false;
        m_remote->requestRemoteBoardState();
    }
}

void CompositeBoard::sendLocalBoardState()
{
    if (m_remote) {
        m_hasLocalMoves = false;
        m_remote->setBoardState(m_local);
    }
}

void CompositeBoard::requestNewGame()
{
    m_drawRequested = false;
    m_local = BoardState::newGame();
    if (m_remote)
        m_remote->requestNewGame();
    emit boardStateChanged(m_local);
}

void CompositeBoard::requestNewGameOptions(const Chessboard::GameOptions& gameOptions)
{
    m_drawRequested = false;
    m_local = BoardState::newGame();
    if (m_remote)
        m_remote->requestNewGame(gameOptions);
    emit boardStateChanged(m_local);
}

void CompositeBoard::requestPromotion(Piece piece)
{
    m_drawRequested = false;
    bool ok = m_local.promote(piece);
    if (m_remote)
        m_remote->requestPromotion(piece);
    else
        m_hasLocalMoves = true;
    if (ok)
        emit boardStateChanged(m_local);
    checkGameOver();
}

void CompositeBoard::requestDraw(Colour requestor)
{
    if (m_remote) {
        m_remote->requestDraw(requestor);
    } else {
        m_hasLocalMoves = true;
        DrawReason reason = DrawReason::None;
        if (m_local.isClaimableDraw(&reason)) {
            m_drawRequested = false;
            emit draw(reason);
        } else if (m_drawRequested && m_drawRequestor != requestor) {
            m_drawRequested = false;
            emit draw(DrawReason::MutualAgreement);
        } else {
            m_drawRequested = true;
            emit drawRequested(requestor);
        }
    }
}

void CompositeBoard::requestResignation(Colour requestor)
{
    m_drawRequested = false;
    if (m_remote) {
        m_remote->requestResignation(requestor);
    } else {
        m_hasLocalMoves = true;
        emit resignation(requestor);
    }
}
