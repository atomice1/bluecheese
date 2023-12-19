#ifndef STOCKFISHAIPLAYER_H
#define STOCKFISHAIPLAYER_H

#include <QProcess>
#include <QString>
#include "aiplayer.h"

class StockfishAiPlayer : public AiPlayer
{
public:
    StockfishAiPlayer(Chessboard::Colour colour, const QString& stockfishPath, QObject *parent);
    ~StockfishAiPlayer();
    void start(const Chessboard::BoardState& state) override;
    void promotionRequired() override;
    void drawRequested() override;
private:
    bool initialize();
    void sendCommand(const QByteArray& command);
    QByteArray waitForResponse(const QByteArray& response);
    void processResponse(const QByteArray& response);

    QProcess *m_process;
    QString m_stockfishPath;
    bool m_initialized {};
};

#endif // STOCKFISHAIPLAYER_H
