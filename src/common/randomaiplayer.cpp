#include <QRandomGenerator>
#include "randomaiplayer.h"

RandomAiPlayer::RandomAiPlayer(Chessboard::Colour colour, QObject *parent) :
    AiPlayer(colour, parent)
{
}

void RandomAiPlayer::start(const Chessboard::BoardState& state)
{
    qDebug("RandomAiPlayer::start");
    auto moves = state.legalMoves();
    auto n = QRandomGenerator::global()->bounded(0, moves.size());
    emit requestMove(moves.at(n).first.row,
                     moves.at(n).first.col,
                     moves.at(n).second.row,
                     moves.at(n).second.col);
}

void RandomAiPlayer::promotionRequired()
{
    emit requestPromotion(Chessboard::Piece::Queen);
}

void RandomAiPlayer::drawRequested()
{
}
