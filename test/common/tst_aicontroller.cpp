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
#include "aiplayerfactory.h"
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

class SequentialAiPlayer : public FakeAiPlayer
{
    Q_OBJECT
public:
    SequentialAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) :
        FakeAiPlayer(colour, parent)
    {
    }
    void start(const Chessboard::BoardState& state) override
    {
        switch(colour()) {
        case Chessboard::Colour::White:
            emit requestMove(1, startCallCount, 2, startCallCount);
            break;
        case Chessboard::Colour::Black:
            emit requestMove(5, startCallCount, 4, startCallCount);
            break;
        }
        startCallCount++;
    }
private:
    int startCallCount {};
};

template<typename White, typename Black=White>
class TestAiPlayerFactory : public AiPlayerFactory
{
public:
    AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) override
    {
        switch (colour) {
        case Chessboard::Colour::White:
            m_whiteAiPlayer = new White(colour, parent);
            return m_whiteAiPlayer;
        case Chessboard::Colour::Black:
            m_blackAiPlayer = new Black(colour, parent);
            return m_blackAiPlayer;
        }
    }
    White *whiteAiPlayer() const { return m_whiteAiPlayer; }
    Black *blackAiPlayer() const { return m_blackAiPlayer; }
private:
    White *m_whiteAiPlayer {};
    Black *m_blackAiPlayer {};
};

class TestAiController: public QObject
{
    Q_OBJECT
private slots:
    void hungAiPlayerIsNotDestroyed()
    {
        TestAiPlayerFactory<HungAiPlayer> hungAiPlayerFactory;
        std::unique_ptr<AiController> controller(new AiController(&hungAiPlayerFactory));
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
        controller->start(Chessboard::Colour::White, board);
        QVERIFY(requestMoveSpy.wait());
        QThread *thread = hungAiPlayerFactory.whiteAiPlayer()->thread();
        QVERIFY(thread != nullptr);
        QSignalSpy threadDestroyedSpy(thread, &QThread::destroyed);
        QSignalSpy whiteAiPlayerDestroyedSpy(hungAiPlayerFactory.whiteAiPlayer(), &AiPlayer::destroyed);
        QSignalSpy blackAiPlayerDestroyedSpy(hungAiPlayerFactory.blackAiPlayer(), &AiPlayer::destroyed);
        controller.reset();
        threadDestroyedSpy.wait(200);
        QCOMPARE(threadDestroyedSpy.count(), 0);
        QCOMPARE(whiteAiPlayerDestroyedSpy.count(), 0);
        QCOMPARE(blackAiPlayerDestroyedSpy.count(), 0);
    }

    void goodAiPlayerIsDestroyed()
    {
        TestAiPlayerFactory<FakeAiPlayer> fakeAiPlayerFactory;
        std::unique_ptr<AiController> controller(new AiController(&fakeAiPlayerFactory));
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
        controller->start(Chessboard::Colour::White, board);
        QVERIFY(requestMoveSpy.wait());
        QThread *thread = fakeAiPlayerFactory.whiteAiPlayer()->thread();
        QVERIFY(thread != nullptr);
        QSignalSpy threadDestroyedSpy(thread, &QThread::destroyed);
        QSignalSpy whiteAiPlayerDestroyedSpy(fakeAiPlayerFactory.whiteAiPlayer(), &AiPlayer::destroyed);
        QSignalSpy blackAiPlayerDestroyedSpy(fakeAiPlayerFactory.blackAiPlayer(), &AiPlayer::destroyed);
        controller.reset();
        QVERIFY(threadDestroyedSpy.wait());
        QCOMPARE(whiteAiPlayerDestroyedSpy.count(), 1);
        QCOMPARE(blackAiPlayerDestroyedSpy.count(), 1);
    }

    void cancelResets()
    {
        TestAiPlayerFactory<CancelAiPlayer> cancelAiPlayerFactory;
        std::unique_ptr<AiController> controller(new AiController(&cancelAiPlayerFactory));
        Chessboard::BoardState board = Chessboard::BoardState::newGame();
        QSignalSpy cancelledSpy(cancelAiPlayerFactory.whiteAiPlayer(), &CancelAiPlayer::cancelled);
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

    // If the main thread calls start / cancel / start, check it does not see
    // a requestMove from the first start.
    void cancelIsSerialized()
    {
        for (int i=0;i<100;++i) {
            TestAiPlayerFactory<SequentialAiPlayer> sequentialAiPlayerFactory;
            std::unique_ptr<AiController> controller(new AiController(&sequentialAiPlayerFactory));
            QSignalSpy requestMoveSpy(controller.get(), &AiController::requestMove);
            Chessboard::BoardState board = Chessboard::BoardState::newGame();
            controller->start(Chessboard::Colour::White, board);
            controller->cancel();
            controller->start(Chessboard::Colour::White, board);
            controller->cancel();
            controller->start(Chessboard::Colour::White, board);
            controller->cancel();
            controller->start(Chessboard::Colour::White, board);
            if (requestMoveSpy.isEmpty())
                QVERIFY(requestMoveSpy.wait());
            QList<QVariant> arguments = requestMoveSpy.takeFirst();
            QCOMPARE(arguments.at(1).value<int>(), 3);
            controller->start(Chessboard::Colour::White, board);
            controller->cancel();
            controller->start(Chessboard::Colour::White, board);
            controller->cancel();
            controller->start(Chessboard::Colour::White, board);
            if (requestMoveSpy.count() < 2)
                QVERIFY(requestMoveSpy.wait());
            arguments = requestMoveSpy.takeFirst();
            QCOMPARE(arguments.at(1).value<int>(), 6);
        }
    }
};

QTEST_MAIN(TestAiController)

#include "tst_aicontroller.moc"
