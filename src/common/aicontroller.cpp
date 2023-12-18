#include <QThreadPool>
#include "aicontroller.h"
#include "aiplayer.h"

AiController::AiController(QObject *parent)
    : QObject{parent},
      m_thread(new QThread(this))
{}

//! This class takes ownership of aiPlayer
void AiController::setAiPlayer(Chessboard::Colour colour, AiPlayer *aiPlayer)
{
    AiPlayer **ref = aiPlayerRef(colour);
    if (*ref) {
        (*ref)->cancel();
        disconnect(*ref, &AiPlayer::requestMove, this, &AiController::requestMove);
        disconnect(*ref, &AiPlayer::requestDraw, this, &AiController::requestDraw);
        disconnect(*ref, &AiPlayer::requestResignation, this, &AiController::requestResignation);
        disconnect(*ref, &AiPlayer::requestPromotion, this, &AiController::requestPromotion);
    }
    *ref = aiPlayer;
    if (*ref) {
        connect(*ref, &AiPlayer::requestMove, this, &AiController::requestMove);
        connect(*ref, &AiPlayer::requestDraw, this, &AiController::requestDraw);
        connect(*ref, &AiPlayer::requestResignation, this, &AiController::requestResignation);
        connect(*ref, &AiPlayer::requestPromotion, this, &AiController::requestPromotion);
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
