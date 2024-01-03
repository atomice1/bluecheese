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
#include <QThread>
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
    if (m_process && m_process->processId() != 0) {
        QThread *processKillerThread = new QThread;
        m_process->setParent(nullptr);
        m_process->moveToThread(processKillerThread);
        connect(m_process, &QProcess::finished, m_process, &QProcess::deleteLater);
        connect(m_process, &QProcess::destroyed, processKillerThread, &QThread::deleteLater);
        sendCommand("quit");
        QProcess *process = m_process;
        QMetaObject::invokeMethod(m_process, [process]() {
                process->write("quit\n");
            }, Qt::QueuedConnection);
    }
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
    sendCommand("setoption name OwnBook value true");
    return true;
}

void StockfishAiPlayer::sendCommand(const QByteArray& command)
{
    qDebug("sendCommand: %s", command.constData());
    if (!m_initialized) {
        m_initialized = true;
        if (!initialize())
            return;
    }
    if (m_process)
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
    QByteArray command = section(response, 0);
    if (command == "bestmove") {
        QByteArray move = section(response, 1);
        if (m_assistanceMode) {
            if (m_currentMove.isEmpty())
                m_bestMove = move.left(4);
            nextAssistance();
        } else {
            Chessboard::AlgebraicNotation an = Chessboard::AlgebraicNotation::fromString(QString::fromLatin1(move));
            emit requestMove(an.fromRow, an.fromCol, an.toRow, an.toCol);
            if (an.promotion)
                emit requestPromotion(an.promotionPiece);
        }
    } else if (command == "info" && m_assistanceMode) {
        QList<QByteArray> infos = response.split(' ');
        int index = infos.indexOf("score");
        if (index != -1) {
            if (infos[index + 1] == "mate")
                m_currentScore = infos[index + 2].toInt() * 10000;
            else
                m_currentScore = infos[index + 2].toInt();
            qDebug("score: %d", m_currentScore);
        }
    }
}

void StockfishAiPlayer::start(const Chessboard::BoardState& state)
{
    qDebug("StockfishAiPlayer::start");
    m_assistanceMode = false;
    sendCommand("stop");
    sendCommand("isready");
    if (waitForResponse("readyok").isNull())
        return;
    sendCommand("position fen " + state.toFenString().toLatin1());
    sendCommand("isready");
    if (waitForResponse("readyok").isNull())
        return;
    sendCommand("go movetime " + QByteArray::number(m_elo * m_elo / 2000));
}

void StockfishAiPlayer::cancel()
{
    QMetaObject::invokeMethod(this, [this]() {
            if (m_process)
                sendCommand("stop");
        }, Qt::QueuedConnection);
}

void StockfishAiPlayer::promotionRequired()
{
    emit requestPromotion(Chessboard::Piece::Queen);
}

void StockfishAiPlayer::setStrength(int elo)
{
    sendCommand("setoption name UCI_LimitStrength value true");
    sendCommand("setoption name UCI_Elo value " + QByteArray::number(elo));
    m_elo = elo;
}

void StockfishAiPlayer::setAssistanceLevel(int level)
{
    m_assistanceLevel = level;
}

void StockfishAiPlayer::startAssistance(const Chessboard::BoardState& state)
{
    qDebug("StockfishAiPlayer::startAssistance -- level = %d", m_assistanceLevel);
    if (m_assistanceLevel == 1)
        return;
    sendCommand("stop");
    m_assistanceMode = true;
    m_board = state;
    m_sortedMoves = state.sortedLegalMoves();
    m_timePerMove = 400 / m_sortedMoves.size();
    m_currentMove.clear();
    sendCommand("position fen " + m_board.toFenString().toLatin1());
    sendCommand("go movetime " + QByteArray::number(m_timePerMove));
}

// Assistance thresholds in centipawns relative to initial position.
// 2: red = <= -300 cp; green >= -299 cp
// 3: red = <= -300 cp; green >= -100 cp
// 4: red = <= -300 cp; green >= 0 cp
// 5: red = <= -100 cp; green >= 300 cp
// 6: red = <=  -1 cp;  green = best move only
namespace {
    int redThreshold[] =   { 0, -300, -300, -300, -100, -1 };
    int greenThreshold[] = { 0, -299, -100, 0,    300,  0 };
}

void StockfishAiPlayer::nextAssistance()
{
    if (!m_currentMove.isEmpty()) {
        if (m_currentMove == m_bestMove) {
            m_assistanceColours.append(Chessboard::AssistanceColour::Green);
        } else if (m_assistanceLevel != 6 && m_currentScore >= greenThreshold[m_assistanceLevel - 1]) {
            m_assistanceColours.append(Chessboard::AssistanceColour::Green);
        } else if (m_currentScore <= redThreshold[m_assistanceLevel - 1]) {
            m_assistanceColours.append(Chessboard::AssistanceColour::Red);
        } else {
            m_assistanceColours.append(Chessboard::AssistanceColour::Blue);
        }
    }
    if (m_sortedMoves.isEmpty()) {
        if (!m_assistanceColours.isEmpty())
            emit assistance(m_assistanceColours);
        m_assistanceColours.clear();
        return;
    }
    QPair<Chessboard::Square, Chessboard::Square> move = m_sortedMoves.takeFirst();
    m_currentMove = move.first.toString().toLatin1() + move.second.toString().toLatin1();
    if (m_currentMove == m_bestMove) {
        nextAssistance();
    } else {
        sendCommand("position fen " + m_board.toFenString().toLatin1());
        sendCommand("go movetime " + QByteArray::number(m_timePerMove) + " searchmoves " + m_currentMove);
    }
}
