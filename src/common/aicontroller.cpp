#include "aicontroller.h"

AiController::AiController(QObject *parent)
    : QObject{parent}
{}

void AiController::start(const Chessboard::BoardState&)
{
    qWarning("TODO: AiController::start");
}

void AiController::cancel()
{
    qWarning("TODO: AiController::cancel");
}

void AiController::drawRequested(Chessboard::Colour)
{
    qWarning("TODO: AiController::drawRequested");
}

void AiController::promotionRequired()
{
    qWarning("TODO: AiController::promotionRequired");
}
