#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <QObject>
#include "chessboard.h"

class AiController : public QObject
{
    Q_OBJECT
public:
    explicit AiController(QObject *parent = nullptr);

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);

public slots:
    void start(const Chessboard::BoardState& state);
    void cancel();
    void drawRequested(Chessboard::Colour requestor);
    void promotionRequired();
};

#endif // AICONTROLLER_H
