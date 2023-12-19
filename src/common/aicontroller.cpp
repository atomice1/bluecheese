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

#include <QThreadPool>
#include "aicontroller.h"
#include "aiplayer.h"

AiController::AiController(QObject *parent)
    : QObject{parent},
      m_thread(new QThread)
{
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}

AiController::~AiController()
{
    if (m_aiPlayer[0])
        QMetaObject::invokeMethod(m_aiPlayer[0], &AiPlayer::deleteLater, Qt::QueuedConnection);
    if (m_aiPlayer[1])
        QMetaObject::invokeMethod(m_aiPlayer[1], &AiPlayer::deleteLater, Qt::QueuedConnection);
    QObject *threadKiller = new QObject;
    threadKiller->moveToThread(m_thread);
    connect(threadKiller, &QObject::destroyed, m_thread, &QThread::quit);
    QMetaObject::invokeMethod(threadKiller, [threadKiller]() {
            threadKiller->deleteLater();
        }, Qt::QueuedConnection);
}

//! This class takes ownership of aiPlayer
void AiController::setAiPlayer(Chessboard::Colour colour, AiPlayer *aiPlayer)
{
    AiPlayer **ref = aiPlayerRef(colour);
    AiPlayer *oldAiPlayer = *ref;
    if (oldAiPlayer) {
        oldAiPlayer->cancel();
        disconnect(oldAiPlayer, &AiPlayer::requestMove, this, &AiController::requestMove);
        disconnect(oldAiPlayer, &AiPlayer::requestDraw, this, &AiController::requestDraw);
        disconnect(oldAiPlayer, &AiPlayer::requestResignation, this, &AiController::requestResignation);
        disconnect(oldAiPlayer, &AiPlayer::requestPromotion, this, &AiController::requestPromotion);
        QMetaObject::invokeMethod(oldAiPlayer, &AiPlayer::deleteLater, Qt::QueuedConnection);
    }
    *ref = aiPlayer;
    if (aiPlayer) {
        aiPlayer->setParent(nullptr);
        aiPlayer->moveToThread(m_thread);
        connect(aiPlayer, &AiPlayer::requestMove, this, &AiController::requestMove, Qt::QueuedConnection);
        connect(aiPlayer, &AiPlayer::requestDraw, this, &AiController::requestDraw, Qt::QueuedConnection);
        connect(aiPlayer, &AiPlayer::requestResignation, this, &AiController::requestResignation, Qt::QueuedConnection);
        connect(aiPlayer, &AiPlayer::requestPromotion, this, &AiController::requestPromotion, Qt::QueuedConnection);
    }
}

void AiController::cancel()
{
    cancel(Chessboard::Colour::White);
    cancel(Chessboard::Colour::Black);
}

void AiController::cancel(Chessboard::Colour colour)
{
    AiPlayer *aiPlayer = this->aiPlayer(colour);
    if (aiPlayer)
        aiPlayer->cancel();
}

void AiController::start(Chessboard::Colour colour, const Chessboard::BoardState& state)
{
    AiPlayer *aiPlayer = this->aiPlayer(colour);
    if (aiPlayer) {
        QMetaObject::invokeMethod(aiPlayer, [aiPlayer, state]() {
                aiPlayer->start(state);
            }, Qt::QueuedConnection);
    }
}

void AiController::drawRequested(Chessboard::Colour colour)
{
    AiPlayer *aiPlayer = this->aiPlayer(invertColour(colour));
    if (aiPlayer) {
        QMetaObject::invokeMethod(aiPlayer, [aiPlayer]() {
                aiPlayer->drawRequested();
            }, Qt::QueuedConnection);
    }
}

void AiController::promotionRequired(Chessboard::Colour colour)
{
    AiPlayer *aiPlayer = this->aiPlayer(invertColour(colour));
    if (aiPlayer) {
        QMetaObject::invokeMethod(aiPlayer, [aiPlayer]() {
                aiPlayer->promotionRequired();
            }, Qt::QueuedConnection);
    }
}

AiPlayer **AiController::aiPlayerRef(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return &m_aiPlayer[0];
    case Chessboard::Colour::Black:
    default:
        return &m_aiPlayer[1];
    }
}

AiPlayer *AiController::aiPlayer(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return m_aiPlayer[0];
    case Chessboard::Colour::Black:
        default:
        return m_aiPlayer[1];
    }
}
