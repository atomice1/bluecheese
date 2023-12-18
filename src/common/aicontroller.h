#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <QObject>
#include "aiplayer.h"
#include "chessboard.h"

class AiPlayer;

class AiController : public QObject
{
    Q_OBJECT
public:
    explicit AiController(QObject *parent = nullptr);
    void setAiPlayer(Chessboard::Colour colour, AiPlayer *aiPlayer);

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor);
    void requestPromotion(Chessboard::Piece piece);

public slots:
    void start(Chessboard::Colour colour, const Chessboard::BoardState& state);
    void cancel();
    void cancel(Chessboard::Colour colour);
    void drawRequested(Chessboard::Colour requestor);
    void promotionRequired(Chessboard::Colour colour);

private:
    AiPlayer **aiPlayerRef(Chessboard::Colour);
    AiPlayer *aiPlayer(Chessboard::Colour);

    QThread *m_thread;
    AiPlayer *m_aiPlayer[2] {};
};

#endif // AICONTROLLER_H
