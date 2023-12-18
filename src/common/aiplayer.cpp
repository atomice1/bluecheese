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
}
