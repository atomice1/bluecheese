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
    void startAssistance(const Chessboard::BoardState& state) override;
    void setAssistanceLevel(int level) override;
private slots:
    void readyReadFromEngine();
private:
    bool initialize();
    void sendCommand(const QByteArray& command);
    QByteArray waitForResponse(const QByteArray& response);
    void processResponse(const QByteArray& response);
    void nextAssistance();

    QProcess *m_process {};
    QString m_stockfishPath;
    Chessboard::BoardState m_board;
    QList<QPair<Chessboard::Square, Chessboard::Square> > m_sortedMoves;
    QList<Chessboard::AssistanceColour> m_assistanceColours;
    QByteArray m_currentMove;
    QByteArray m_bestMove;
    int m_elo { 1000 };
    int m_assistanceLevel {1};
    int m_timePerMove;
    int m_currentScore;
    bool m_initialized {};
    bool m_waitingForResponse {};
    bool m_assistanceMode {};
};

#endif // STOCKFISHAIPLAYER_H
