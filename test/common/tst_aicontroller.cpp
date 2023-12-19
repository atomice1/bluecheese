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

#include <QSignalSpy>
#include <QTest>

#include "aicontroller.h"
#include "chessboard.h"

class FakeAiPlayer : public AiPlayer
{
public:
    FakeAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) :
        AiPlayer(colour, parent)
    {
    }
    void start(const Chessboard::BoardState&) override
    {
        qDebug("emit requestMove");
        emit requestMove(0, 0, 1, 1);
    }
    void promotionRequired() override
    {
    }
    void drawRequested() override
    {
    }
};

class HungAiPlayer : public FakeAiPlayer
{
public:
    HungAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) :
        FakeAiPlayer(colour, parent)
    {
    }
    void start(const Chessboard::BoardState& state) override
    {
        FakeAiPlayer::start(state);
        for (;;) {}
    }
};

class CancelAiPlayer : public FakeAiPlayer
{
    Q_OBJECT
public:
    CancelAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) :
        FakeAiPlayer(colour, parent)
    {
    }
    void start(const Chessboard::BoardState& state) override
    {
        if (startCallCount++ == 0) {
            while (!isCancelled()) {}
            emit cancelled();
        } else {
            if (isCancelled())
                emit cancelled();
            else
                FakeAiPlayer::start(state);
        }
    }
signals:
    void cancelled();
private:
    int startCallCount {};
};

class TestAiController: public QObject
{
    Q_OBJECT
private slots:
    void hungAiPlayerIsNotDestroyed()
    {
        std::unique_ptr<AiController> controller(new AiController);
        std::unique_ptr<QObject> parent(new QObject);
        AiPlayer *whiteAiPlayer = new HungAiPlayer(Chessboard::Colour::Black, parent.get());
        AiPlayer *blackAiPlayer = new HungAiPlayer( Chessboard::Colour::White, parent.get());
        controller->setAiPlayer(Chessboard::Colour::White, whiteAiPlayer);
        controller->setAiPlayer(Chessboard::Colour::Black, blackAiPlayer);
        parent.reset();
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
        controller->start(Chessboard::Colour::White, board);
        QVERIFY(requestMoveSpy.wait());
        QThread *thread = whiteAiPlayer->thread();
        QVERIFY(thread != nullptr);
        QSignalSpy threadDestroyedSpy(thread, &QThread::destroyed);
        QSignalSpy whiteAiPlayerDestroyedSpy(whiteAiPlayer, &AiPlayer::destroyed);
        QSignalSpy blackAiPlayerDestroyedSpy(blackAiPlayer, &AiPlayer::destroyed);
        controller.reset();
        threadDestroyedSpy.wait(200);
        QCOMPARE(threadDestroyedSpy.count(), 0);
        QCOMPARE(whiteAiPlayerDestroyedSpy.count(), 0);
        QCOMPARE(blackAiPlayerDestroyedSpy.count(), 0);
    }

    void goodAiPlayerIsDestroyed()
    {
        std::unique_ptr<AiController> controller(new AiController);
        std::unique_ptr<QObject> parent(new QObject);
        AiPlayer *whiteAiPlayer = new FakeAiPlayer(Chessboard::Colour::Black, parent.get());
        AiPlayer *blackAiPlayer = new FakeAiPlayer(Chessboard::Colour::White, parent.get());
        controller->setAiPlayer(Chessboard::Colour::White, whiteAiPlayer);
        controller->setAiPlayer(Chessboard::Colour::Black, blackAiPlayer);
        parent.reset();
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
        controller->start(Chessboard::Colour::White, board);
        QVERIFY(requestMoveSpy.wait());
        QThread *thread = whiteAiPlayer->thread();
        QVERIFY(thread != nullptr);
        QSignalSpy threadDestroyedSpy(thread, &QThread::destroyed);
        QSignalSpy whiteAiPlayerDestroyedSpy(whiteAiPlayer, &AiPlayer::destroyed);
        QSignalSpy blackAiPlayerDestroyedSpy(blackAiPlayer, &AiPlayer::destroyed);
        controller.reset();
        QVERIFY(threadDestroyedSpy.wait());
        QCOMPARE(whiteAiPlayerDestroyedSpy.count(), 1);
        QCOMPARE(blackAiPlayerDestroyedSpy.count(), 1);
    }

    void cancelResets()
    {
        std::unique_ptr<AiController> controller(new AiController);
        CancelAiPlayer *whiteAiPlayer = new CancelAiPlayer(Chessboard::Colour::Black);
        CancelAiPlayer *blackAiPlayer = new CancelAiPlayer(Chessboard::Colour::White);
        controller->setAiPlayer(Chessboard::Colour::White, whiteAiPlayer);
        controller->setAiPlayer(Chessboard::Colour::Black, blackAiPlayer);
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy cancelledSpy(whiteAiPlayer, &CancelAiPlayer::cancelled);
        controller->start(Chessboard::Colour::White, board);
        QCOMPARE(cancelledSpy.count(), 0);
        controller->cancel();
        QVERIFY(cancelledSpy.wait());
        QCOMPARE(cancelledSpy.count(), 1);
        QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
        controller->start(Chessboard::Colour::White, board);
        QVERIFY(requestMoveSpy.wait());
        QCOMPARE(cancelledSpy.count(), 1);
    }
};

QTEST_MAIN(TestAiController)

#include "tst_aicontroller.moc"
