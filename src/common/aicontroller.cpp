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

#include <QThreadPool>
#include "aicontroller.h"
#include "aiplayer.h"
#include "aiplayerfactory.h"

class AiPlayerWorkerProxy : public QObject
{
    Q_OBJECT
public:
    explicit AiPlayerWorkerProxy(AiPlayer *aiPlayer, QObject *parent = nullptr) :
        QObject(parent),
        m_aiPlayer(aiPlayer)
    {
        aiPlayer->setParent(this);
        connect(m_aiPlayer, &AiPlayer::requestMove, this, &AiPlayerWorkerProxy::requestMove);
        connect(m_aiPlayer, &AiPlayer::requestDraw, this, &AiPlayerWorkerProxy::requestDraw);
        connect(m_aiPlayer, &AiPlayer::declineDraw, this, &AiPlayerWorkerProxy::declineDraw);
        connect(m_aiPlayer, &AiPlayer::requestResignation, this, &AiPlayerWorkerProxy::requestResignation);
        connect(m_aiPlayer, &AiPlayer::requestPromotion, this, &AiPlayerWorkerProxy::requestPromotion);
        connect(m_aiPlayer, &AiPlayer::assistance, this, &AiPlayerWorkerProxy::assistance);
        connect(m_aiPlayer, &AiPlayer::error, this, &AiPlayerWorkerProxy::error);
    }
signals:
    void requestMoveSerial(long serial, int fromRow, int fromCol, int toRow, int toCol);
    void requestDrawSerial(long serial);
    void declineDrawSerial(long serial);
    void requestResignationSerial(long serial);
    void requestPromotionSerial(long serial, Chessboard::Piece piece);
    void assistanceSerial(long serial, QList<Chessboard::AssistanceColour> colours);
    void error(AiPlayer::Error error);
public slots:
    void startSerial(long serial, const Chessboard::BoardState& state)
    {
        m_serial = serial;
        m_aiPlayer->start(state);
    }
    void promotionRequiredSerial(long serial)
    {
        m_serial = serial;
        m_aiPlayer->promotionRequired();
    }
    void drawRequestedSerial(long serial)
    {
        m_serial = serial;
        m_aiPlayer->drawRequested();
    }
    void drawDeclinedSerial(long serial)
    {
        m_serial = serial;
        m_aiPlayer->drawDeclined();
    }
    void setStrength(int elo)
    {
        m_aiPlayer->setStrength(elo);
    }
    void setAssistanceLevel(int level)
    {
        m_aiPlayer->setAssistanceLevel(level);
    }
    void startAssistanceSerial(long serial, const Chessboard::BoardState& state)
    {
        m_serial = serial;
        m_aiPlayer->startAssistance(state);
    }
    void cancel()
    {
        m_aiPlayer->cancel();
    }
private slots:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol)
    {
        emit requestMoveSerial(m_serial, fromRow, fromCol, toRow, toCol);
    }
    void requestDraw()
    {
        emit requestDrawSerial(m_serial);
    }
    void declineDraw()
    {
        emit declineDrawSerial(m_serial);
    }
    void requestResignation()
    {
        emit requestResignationSerial(m_serial);
    }
    void requestPromotion(Chessboard::Piece piece)
    {
        emit requestPromotionSerial(m_serial, piece);
    }
    void assistance(const QList<Chessboard::AssistanceColour>& colours)
    {
        emit assistanceSerial(m_serial, colours);
    }
private:
    AiPlayer *m_aiPlayer;
    long m_serial {};
};

class AiPlayerControllerProxy : public QObject
{
    Q_OBJECT
public:
    AiPlayerControllerProxy(AiPlayerWorkerProxy *worker, QObject *parent = nullptr) :
        QObject(parent),
        m_worker(worker)
    {
        connect(m_worker, &AiPlayerWorkerProxy::requestMoveSerial, this, &AiPlayerControllerProxy::requestMoveSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::requestDrawSerial, this, &AiPlayerControllerProxy::requestDrawSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::declineDrawSerial, this, &AiPlayerControllerProxy::declineDrawSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::requestResignationSerial, this, &AiPlayerControllerProxy::requestResignationSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::requestPromotionSerial, this, &AiPlayerControllerProxy::requestPromotionSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::assistanceSerial, this, &AiPlayerControllerProxy::assistanceSerial, Qt::QueuedConnection);
        connect(m_worker, &AiPlayerWorkerProxy::error, this, &AiPlayerControllerProxy::error, Qt::QueuedConnection);
        connect(this, &QObject::destroyed, m_worker, &QObject::deleteLater);
    }

signals:
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestDraw();
    void declineDraw();
    void requestResignation();
    void requestPromotion(Chessboard::Piece piece);
    void assistance(QList<Chessboard::AssistanceColour> colours);
    void error(AiPlayer::Error error);

public slots:
    void start(const Chessboard::BoardState& state)
    {
        long serial = ++m_serial;
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, serial, state]() {
                worker->startSerial(serial, state);
            }, Qt::QueuedConnection);
    }
    void promotionRequired()
    {
        long serial = ++m_serial;
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, serial]() {
                worker->promotionRequiredSerial(serial);
            }, Qt::QueuedConnection);
    }
    void drawRequested()
    {
        long serial = ++m_serial;
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, serial]() {
                worker->drawRequestedSerial(serial);
            }, Qt::QueuedConnection);
    }
    void drawDeclined()
    {
        long serial = ++m_serial;
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, serial]() {
                worker->drawDeclinedSerial(serial);
            }, Qt::QueuedConnection);
    }
    void setStrength(int elo)
    {
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, elo]() {
                worker->setStrength(elo);
            }, Qt::QueuedConnection);
    }
    void setAssistanceLevel(int level)
    {
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, level]() {
                worker->setAssistanceLevel(level);
            }, Qt::QueuedConnection);
    }
    void startAssistance(const Chessboard::BoardState& state)
    {
        long serial = ++m_serial;
        AiPlayerWorkerProxy *worker = m_worker;
        QMetaObject::invokeMethod(worker, [worker, serial, state]() {
                worker->startAssistanceSerial(serial, state);
            }, Qt::QueuedConnection);
    }
    void cancel()
    {
        m_worker->cancel();
    }

private slots:
    void requestMoveSerial(long serial, int fromRow, int fromCol, int toRow, int toCol)
    {
        if (serial == m_serial)
            emit requestMove(fromRow, fromCol, toRow, toCol);
    }
    void requestDrawSerial(long serial)
    {
        if (serial == m_serial)
            emit requestDraw();
    }
    void declineDrawSerial(long serial)
    {
        if (serial == m_serial)
            emit declineDraw();
    }
    void requestResignationSerial(long serial)
    {
        if (serial == m_serial)
            emit requestResignation();
    }
    void requestPromotionSerial(long serial, Chessboard::Piece piece)
    {
        if (serial == m_serial)
            emit requestPromotion(piece);
    }
    void assistanceSerial(long serial, const QList<Chessboard::AssistanceColour>& colours)
    {
        if (serial == m_serial)
            emit assistance(colours);
    }

private:
    AiPlayerWorkerProxy *m_worker;
    long m_serial {};
};

AiController::AiController(AiPlayerFactory *factory, QObject *parent)
    : QObject{parent},
      m_thread(new QThread)
{
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
    createAiPlayer(Chessboard::Colour::White, factory, &m_whiteAiPlayer);
    createAiPlayer(Chessboard::Colour::Black, factory, &m_blackAiPlayer);
}

AiController::~AiController()
{
    // The controller proxies must be deleted before the lambda is queued
    // to delete the thread as deleting the controller proxies will
    // queue the deletion of the worker proxies, which must happen
    // before the thread is destroyed.
    delete m_whiteAiPlayer;
    delete m_blackAiPlayer;
    QObject *threadKiller = new QObject;
    threadKiller->moveToThread(m_thread);
    connect(threadKiller, &QObject::destroyed, m_thread, &QThread::quit);
    QMetaObject::invokeMethod(threadKiller, [threadKiller]() {
            threadKiller->deleteLater();
        }, Qt::QueuedConnection);
}

void AiController::createAiPlayer(Chessboard::Colour colour, AiPlayerFactory *factory, AiPlayerControllerProxy **controllerProxy)
{
    AiPlayer *aiPlayer = factory->createAiPlayer(colour);
    AiPlayerWorkerProxy *workerProxy = new AiPlayerWorkerProxy(aiPlayer);
    *controllerProxy = new AiPlayerControllerProxy(workerProxy, this);
    connect(*controllerProxy, &AiPlayerControllerProxy::requestMove, this, &AiController::requestMove);
    connect(*controllerProxy, &AiPlayerControllerProxy::requestDraw, this, [this, colour]() {
        emit requestDraw(colour);
    });
    connect(*controllerProxy, &AiPlayerControllerProxy::declineDraw, this, [this, colour]() {
        emit declineDraw(colour);
    });
    connect(*controllerProxy, &AiPlayerControllerProxy::requestResignation, this, [this, colour]() {
        emit requestResignation(colour);
    });
    connect(*controllerProxy, &AiPlayerControllerProxy::requestPromotion, this, &AiController::requestPromotion);
    connect(*controllerProxy, &AiPlayerControllerProxy::assistance, this, &AiController::assistance);
    connect(*controllerProxy, &AiPlayerControllerProxy::error, this, &AiController::error);
    workerProxy->moveToThread(m_thread);
}

void AiController::cancel()
{
    cancel(Chessboard::Colour::White);
    cancel(Chessboard::Colour::Black);
}

void AiController::cancel(Chessboard::Colour colour)
{
    aiPlayer(colour)->cancel();
}

void AiController::start(Chessboard::Colour colour, const Chessboard::BoardState& state)
{
    aiPlayer(colour)->start(state);
}

void AiController::drawRequested(Chessboard::Colour colour)
{
    aiPlayer(invertColour(colour))->drawRequested();
}

void AiController::drawDeclined(Chessboard::Colour colour)
{
    aiPlayer(invertColour(colour))->drawDeclined();
}

void AiController::promotionRequired(Chessboard::Colour colour)
{
    aiPlayer(colour)->promotionRequired();
}

void AiController::setStrength(Chessboard::Colour colour, int elo)
{
    aiPlayer(colour)->setStrength(elo);
}

void AiController::setAssistanceLevel(Chessboard::Colour colour, int level)
{
    aiPlayer(colour)->setAssistanceLevel(level);
}

void AiController::startAssistance(Chessboard::Colour colour, const Chessboard::BoardState& state)
{
    aiPlayer(colour)->startAssistance(state);
}

AiPlayerControllerProxy *AiController::aiPlayer(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return m_whiteAiPlayer;
    case Chessboard::Colour::Black:
        return m_blackAiPlayer;
    }
}

#include "aicontroller.moc"
