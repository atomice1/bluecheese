#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <QObject>
#include "chessboard.h"

class AiPlayer : public QObject {
    Q_OBJECT
public:
    enum Error {
        EngineNotConfigured,
        EngineNotFound,
        EngineIncompatible,
        EngineNoStart,
        EngineNoBoot,
        EngineTimedOut,
        UnknownError
    };

    AiPlayer(Chessboard::Colour colour, QObject *parent);
    virtual ~AiPlayer() {};

    Chessboard::Colour colour() const;
    bool isCancelled() const;
    virtual void cancel();

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor) ;
    void requestPromotion(Chessboard::Piece piece);
    void error(Error error);

public slots:
    virtual void start(const Chessboard::BoardState& state) = 0;
    virtual void promotionRequired() = 0;
    virtual void drawRequested() = 0;

private:
    Chessboard::Colour m_colour;
    bool m_cancelled {};
};

#endif // AIPLAYER_H
