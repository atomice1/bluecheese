#ifndef AIPLAYERFACTORY_H
#define AIPLAYERFACTORY_H

#include "chessboard.h"

class AiPlayer;
class QObject;

class AiPlayerFactory
{
public:
    virtual AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) = 0;
};

#endif // AIPLAYERFACTORY_H
