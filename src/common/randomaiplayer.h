#ifndef RANDOMAIPLAYER_H
#define RANDOMAIPLAYER_H

#include "aiplayer.h"

class RandomAiPlayer : public AiPlayer
{
public:
    RandomAiPlayer(Chessboard::Colour colour, QObject *parent);
    void start(const Chessboard::BoardState& state) override;
    void promotionRequired() override;
    void drawRequested() override;
};

#endif // RANDOMAIPLAYER_H
