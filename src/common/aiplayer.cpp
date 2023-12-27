#include "aiplayer.h"

AiPlayer::AiPlayer(Chessboard::Colour colour, QObject *parent) :
    QObject(parent),
    m_colour(colour)
{
};

Chessboard::Colour AiPlayer::colour() const
{
    return m_colour;
}

bool AiPlayer::isCancelled() const
{
    return m_cancelled;
}

void AiPlayer::cancel()
{
    m_cancelled = true;
    QMetaObject::invokeMethod(this, [this]() {
            m_cancelled = false;
        }, Qt::QueuedConnection);
}

void AiPlayer::drawRequested()
{
    emit declineDraw();
}

void AiPlayer::drawDeclined()
{
}

void AiPlayer::setStrength(int)
{
}

void AiPlayer::startAssistance(const Chessboard::BoardState&)
{
}
