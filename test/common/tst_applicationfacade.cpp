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

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include "aiplayerfactory.h"
#include "applicationfacade.h"
#include "chessboard.h"

using namespace Chessboard;

class MockRemoteBoard : public RemoteBoard
{
public:
    MockRemoteBoard(QObject *parent = nullptr) :
        RemoteBoard(BoardAddress(), parent) {
    }
};

class MockApplicationFacade : public ApplicationFacade
{
public:
    MockApplicationFacade(QObject *parent = nullptr) :
        ApplicationFacade(parent) {}
    MockApplicationFacade(AiPlayerFactory *aiPlayerFactory, QObject *parent = nullptr) :
        ApplicationFacade(aiPlayerFactory, parent) {}
    ConnectionManager *connectionManager()
    {
        return m_connectionManager;
    }
};

template<typename T>
class AutoDeletePointer : public QPointer<T>
{
public:
    AutoDeletePointer(T *ptr) : QPointer<T>(ptr) {}
    ~AutoDeletePointer() { if (*this) delete *this; }
};

class WhitePromotionAiPlayer : public AiPlayer
{
public:
    WhitePromotionAiPlayer(Chessboard::Colour colour, QObject *parent) :
        AiPlayer(colour, parent) {}
    void start(const Chessboard::BoardState& state) override
    {
        qDebug("WhitePromotionAiPlayer::start");
        emit requestMove(6, 0, 7, 0);
        qDebug("WhitePromotionAiPlayer::start ended");
    }
    void promotionRequired() override
    {
        qDebug("WhitePromotionAiPlayer::promotionRequired");
        emit requestPromotion(Chessboard::Piece::Rook);
    }
    void drawRequested() override
    {
    }
};

class BlackPromotionAiPlayer : public AiPlayer
{
    Q_OBJECT
public:
    BlackPromotionAiPlayer(Chessboard::Colour colour, QObject *parent) :
        AiPlayer(colour, parent) {}
    void start(const Chessboard::BoardState& state) override
    {
        qDebug("BlackPromotionAiPlayer::start");
        // Check we do not observe the unpromoted pawn.
        if (state[Square::fromAlgebraicString("a8")] != ColouredPiece(Colour::White, Piece::Rook))
            emit aiStartedTooEarly();
        emit started();
    }
    void promotionRequired() override
    {
    }
    void drawRequested() override
    {
    }
signals:
    void aiStartedTooEarly();
    void started();
};

class PromotionAiPlayerFactory : public AiPlayerFactory
{
public:
    AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) override
    {
        switch (colour) {
        case Chessboard::Colour::White:
            m_whitePromotionAiPlayer = new WhitePromotionAiPlayer(colour, parent);
            return m_whitePromotionAiPlayer;
        case Chessboard::Colour::Black:
            m_blackPromotionAiPlayer = new BlackPromotionAiPlayer(colour, parent);
            return m_blackPromotionAiPlayer;
        }
    }
    WhitePromotionAiPlayer *whitePromotionAiPlayer() const { return m_whitePromotionAiPlayer; }
    BlackPromotionAiPlayer *blackPromotionAiPlayer() const { return m_blackPromotionAiPlayer; }
private:
    WhitePromotionAiPlayer *m_whitePromotionAiPlayer {};
    BlackPromotionAiPlayer *m_blackPromotionAiPlayer {};
};

class TestApplicationFacade: public QObject
{
    Q_OBJECT
private slots:
    void connectDisconnect()
    {
        MockApplicationFacade appFacade;
        AutoDeletePointer<MockRemoteBoard> board = new MockRemoteBoard;
        QSignalSpy connectedSpy(&appFacade, &MockApplicationFacade::connected);
        emit appFacade.connectionManager()->connected(board);
        QCOMPARE(connectedSpy.count(), 1);
        QSignalSpy disconnectedSpy(&appFacade, &MockApplicationFacade::disconnected);
        emit appFacade.connectionManager()->disconnected();
        QCOMPARE(disconnectedSpy.count(), 1);
    }
    void aiPromotion()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Ai;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                             &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        const QString initialState = "3rk3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        if (boardStateChangedSpy.isEmpty())
            QVERIFY(boardStateChangedSpy.wait());
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        if (boardStateChangedSpy.isEmpty())
            QVERIFY(boardStateChangedSpy.wait());
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        QCOMPARE(promotionRequiredSpy.count(), 0);
        if (blackAiStartedSpy.isEmpty())
            QVERIFY(blackAiStartedSpy.wait());
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
    }
    void appHumanPromotion()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                             &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        const QString initialState = "3rk3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        appFacade.requestMove(6, 0, 7, 0);
        QCOMPARE(activeColourChangedSpy.count(), 0);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        QCOMPARE(promotionRequiredSpy.count(), 1);
        appFacade.requestPromotion(Chessboard::Piece::Rook);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        if (blackAiStartedSpy.isEmpty())
            QVERIFY(blackAiStartedSpy.wait());
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
    }
    void boardHumanPromotion()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        AutoDeletePointer<MockRemoteBoard> board = new MockRemoteBoard;
        emit appFacade.connectionManager()->connected(board);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                        &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        const QString initialState = "3rk3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        emit board->remoteMove(6, 0, 7, 0);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        QCOMPARE(activeColourChangedSpy.count(), 0);
        QCOMPARE(promotionRequiredSpy.count(), 0);
        emit board->remotePromotion(Chessboard::Piece::Rook);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        if (blackAiStartedSpy.isEmpty())
            QVERIFY(blackAiStartedSpy.wait());
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
    }
    void aiPromotionGameOver()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Ai;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                             &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        QSignalSpy gameOverSpy(&appFacade, &ApplicationFacade::gameOver);
        const QString initialState = "4k3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        if (boardStateChangedSpy.isEmpty())
            QVERIFY(boardStateChangedSpy.wait());
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        if (boardStateChangedSpy.isEmpty())
            QVERIFY(boardStateChangedSpy.wait());
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        QCOMPARE(promotionRequiredSpy.count(), 0);
        QCOMPARE(blackAiStartedSpy.count(), 0);
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
        QCOMPARE(gameOverSpy.count(), 1);
    }
    void appHumanPromotionGameOver()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                             &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        QSignalSpy gameOverSpy(&appFacade, &ApplicationFacade::gameOver);
        const QString initialState = "4k3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        appFacade.requestMove(6, 0, 7, 0);
        QCOMPARE(activeColourChangedSpy.count(), 0);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        QCOMPARE(promotionRequiredSpy.count(), 1);
        QCOMPARE(gameOverSpy.count(), 0);
        appFacade.requestPromotion(Chessboard::Piece::Rook);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        QCOMPARE(blackAiStartedSpy.count(), 0);
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
        QCOMPARE(gameOverSpy.count(), 1);
    }
    void boardHumanPromotionGameOver()
    {
        PromotionAiPlayerFactory promotionAiPlayerFactory;
        MockApplicationFacade appFacade(&promotionAiPlayerFactory);
        AutoDeletePointer<MockRemoteBoard> board = new MockRemoteBoard;
        emit appFacade.connectionManager()->connected(board);
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy promotionRequiredSpy(&appFacade, &ApplicationFacade::promotionRequired);
        QSignalSpy blackAiStartedTooEarlySpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                             &BlackPromotionAiPlayer::aiStartedTooEarly);
        QSignalSpy blackAiStartedSpy(promotionAiPlayerFactory.blackPromotionAiPlayer(),
                                     &BlackPromotionAiPlayer::started);
        QSignalSpy gameOverSpy(&appFacade, &ApplicationFacade::gameOver);
        const QString initialState = "4k3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
        BoardState state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece::None);
        activeColourChangedSpy.clear();
        emit board->remoteMove(6, 0, 7, 0);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Pawn));
        QCOMPARE(activeColourChangedSpy.count(), 0);
        QCOMPARE(promotionRequiredSpy.count(), 0);
        QCOMPARE(gameOverSpy.count(), 0);
        emit board->remotePromotion(Chessboard::Piece::Rook);
        QCOMPARE(boardStateChangedSpy.count(), 1);
        arguments = boardStateChangedSpy.takeFirst();
        state = arguments.at(0).value<Chessboard::BoardState>();
        QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
        QCOMPARE(activeColourChangedSpy.count(), 1);
        QCOMPARE(blackAiStartedSpy.count(), 0);
        QCOMPARE(blackAiStartedTooEarlySpy.count(), 0);
        QCOMPARE(gameOverSpy.count(), 1);
    }
};

QTEST_MAIN(TestApplicationFacade)

#include "tst_applicationfacade.moc"
