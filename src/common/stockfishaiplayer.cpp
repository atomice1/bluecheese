/*
 * bluecheese
 * Copyright (C) 2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QFile>
#include "stockfishaiplayer.h"

namespace {
    QByteArray section(const QByteArray& line, int index)
    {
        int start = 0;
        int end;
        for (;;) {
            end = line.indexOf(' ', start);
            if (end == -1) {
                end = line.indexOf('\n', start);
                if (end == -1)
                    end = line.length();
            }
            if (index == 0)
                break;
            index--;
            start = end + 1;
        }
        return line.mid(start, end - start);
    }
}

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
    connect(m_process, &QProcess::readyRead, this, &StockfishAiPlayer::readyReadFromEngine);
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
    //sendCommand("setoption name UCI_LimitStrength value true");
    return true;
}

void StockfishAiPlayer::sendCommand(const QByteArray& command)
{
    qDebug("sendCommand: %s", command.constData());
    Q_ASSERT(m_process);
    m_process->write(command + "\n");
}

void StockfishAiPlayer::readyReadFromEngine()
{
    if (m_waitingForResponse)
        return;
    while (m_process->canReadLine()) {
        QByteArray line = m_process->readLine();
        line = line.simplified();
        processResponse(line);
    }
}

QByteArray StockfishAiPlayer::waitForResponse(const QByteArray& response)
{
    m_waitingForResponse = true;
    while (!isCancelled()) {
        if (!m_process->waitForReadyRead()) {
            qDebug("StockfishAiPlayer::waitForResponse: waitForReadyRead timed out");
            emit error(EngineTimedOut);
            return QByteArray();
        }
        while (m_process->canReadLine()) {
            QByteArray line = m_process->readLine();
            line = line.simplified();
            processResponse(line);
            if (section(line, 0) == response) {
                m_waitingForResponse = false;
                return line;
            }
        }
    }
    m_waitingForResponse = false;
    return QByteArray();
}

void StockfishAiPlayer::processResponse(const QByteArray& response)
{
    qDebug("processResponse: %s", response.constData());
    if (section(response, 0) == "bestmove") {
        QByteArray move = section(response, 1);
        Chessboard::AlgebraicNotation an = Chessboard::AlgebraicNotation::fromString(QString::fromLatin1(move));
        emit requestMove(an.fromRow, an.fromCol, an.toRow, an.toCol);
        if (an.promotion)
            emit requestPromotion(an.promotionPiece);
    }
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
    sendCommand("position fen " + state.toFenString().toLatin1());
    sendCommand("isready");
    if (waitForResponse("readyok").isNull())
        return;
    sendCommand("go movetime 200");
}

void StockfishAiPlayer::cancel()
{
    if (m_process)
        sendCommand("stop");
}

void StockfishAiPlayer::promotionRequired()
{
    emit requestPromotion(Chessboard::Piece::Queen);
}

void StockfishAiPlayer::drawRequested()
{
}
