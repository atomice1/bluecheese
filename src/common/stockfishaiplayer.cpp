#include <QFile>
#include "stockfishaiplayer.h"

StockfishAiPlayer::StockfishAiPlayer(Chessboard::Colour colour, const QString& stockfishPath, QObject *parent) :
    AiPlayer(colour, parent),
    m_stockfishPath(stockfishPath)
{
}

StockfishAiPlayer::~StockfishAiPlayer()
{
    if (m_process)
        sendCommand("quit");
}

bool StockfishAiPlayer::initialize()
{
    qDebug("StockfishAiPlayer::initialize");
    if (m_stockfishPath.isEmpty()) {
        emit error(EngineNotConfigured);
        return false;
    }
    if (!QFile::exists(m_stockfishPath)) {
        emit error(EngineNotFound);
        return false;
    }
    m_process = new QProcess(this);
    m_process->start(m_stockfishPath);
    if (!m_process->waitForStarted()) {
        emit error(EngineNoStart);
        return false;
    }
    sendCommand("uci");
    if (waitForResponse("uciok").isNull()) {
        emit error(EngineNoBoot);
        return false;
    }
    return true;
}

void StockfishAiPlayer::sendCommand(const QByteArray& command)
{
    qDebug("sendCommand: %s", command.constData());
    m_process->write(command + "\n");
}

QByteArray StockfishAiPlayer::waitForResponse(const QByteArray& response)
{
    for (;;) {
        if (!m_process->waitForReadyRead()) {
            qDebug("StockfishAiPlayer::waitForResponse: waitForReadyRead timed out");
            emit error(EngineTimedOut);
            return QByteArray();
        }
        while (m_process->canReadLine()) {
            QByteArray line = m_process->readLine();
            line = line.simplified();
            processResponse(line);
            int space = line.indexOf(' ');
            if (space == -1) {
                space = line.indexOf('\n');
                if (space == -1)
                    space = line.length();
            }
            if (line.left(space) == response)
                return line;
        }
    }
}

void StockfishAiPlayer::processResponse(const QByteArray& response)
{
    qDebug("processResponse: %s", response.constData());
}

void StockfishAiPlayer::start(const Chessboard::BoardState& state)
{
    if (!m_initialized) {
        if (!initialize())
            return;
    }
    qDebug("StockfishAiPlayer::start");
    sendCommand("isready");
    if (waitForResponse("readyok").isNull())
        return;
}

void StockfishAiPlayer::promotionRequired()
{
    emit requestPromotion(Chessboard::Piece::Queen);
}

void StockfishAiPlayer::drawRequested()
{
}
