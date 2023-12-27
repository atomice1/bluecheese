#ifndef STOCKFISHAIPLAYER_H
#define STOCKFISHAIPLAYER_H

#include <QProcess>
#include <QString>
#include "aiplayer.h"

class StockfishAiPlayer : public AiPlayer
{
    Q_OBJECT
public:
    StockfishAiPlayer(Chessboard::Colour colour, const QString& stockfishPath, QObject *parent);
    ~StockfishAiPlayer();
    void start(const Chessboard::BoardState& state) override;
    void promotionRequired() override;
    void cancel() override;
    void setStrength(int elo) override;
private slots:
    void readyReadFromEngine();
private:
    bool initialize();
    void sendCommand(const QByteArray& command);
    QByteArray waitForResponse(const QByteArray& response);
    void processResponse(const QByteArray& response);

    QProcess *m_process;
    QString m_stockfishPath;
    int m_elo { 1000 };
    bool m_initialized {};
    bool m_waitingForResponse {};
};

#endif // STOCKFISHAIPLAYER_H
