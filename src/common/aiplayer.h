#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <QSharedData>
#include "chessboard.h"

class AiPlayer : public QObject, public QSharedData {
    Q_OBJECT
public:
    AiPlayer(Chessboard::Colour colour, QObject *parent);

    Chessboard::Colour colour() const;
    bool isCancelled() const;
    void cancel();

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor) ;
    void requestPromotion(Chessboard::Piece piece);

public slots:
    virtual void start(const Chessboard::BoardState& state) = 0;
    virtual void promotionRequired() = 0;
    virtual void drawRequested() = 0;

private:
    Chessboard::Colour m_colour;
    bool m_cancelled {};
};

#endif // AIPLAYER_H
