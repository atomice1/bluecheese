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
    Q_OBJECT
public:
    MockRemoteBoard(QObject *parent = nullptr) :
        RemoteBoard(BoardAddress(), parent) {
    }
    void requestDraw(Colour colour)
    {
        emit remoteReceivedDrawRequested(colour);
    }
signals:
    void remoteReceivedDrawRequested(Colour colour);
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
    AutoDeletePointer() : QPointer<T>() {}
    AutoDeletePointer(T *ptr) : QPointer<T>(ptr) {}
    ~AutoDeletePointer() { if (*this) delete *this; }
    AutoDeletePointer& operator=(T *ptr) { QPointer<T>::operator=(ptr); return *this; };
};

class WhitePromotionAiPlayer : public AiPlayer
{
public:
    WhitePromotionAiPlayer(Chessboard::Colour colour, QObject *parent) :
        AiPlayer(colour, parent) {}
    void start(const Chessboard::BoardState& state) override
    {
        emit requestMove(6, 0, 7, 0);
    }
    void promotionRequired() override
    {
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
        default:
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

class DrawAiPlayer : public AiPlayer
{
    Q_OBJECT
public:
    enum Action {
        RequestDraw,
        AcceptDraw,
        DeclineDraw,
        ClaimDraw,
        AutomaticDraw,
        NoAction
    };

    DrawAiPlayer(Chessboard::Colour colour, Action action, QObject *parent) :
        AiPlayer(colour, parent),
        m_action(action)
    {
    }
    void start(const Chessboard::BoardState& state) override
    {
        emit started();
        Chessboard::DrawReason reason;
        switch (m_action) {
        case RequestDraw:
            emit requestDraw();
            break;
        case ClaimDraw:
            if (state.isClaimableDraw(&reason))
                emit requestDraw();
            else
                emit drawNotClaimable();
        default:
            break;
        }
    }
    void promotionRequired() override
    {
    }
    void drawRequested() override
    {
        switch (m_action) {
        case AcceptDraw:
            emit requestDraw();
            break;
        case DeclineDraw:
            emit declineDraw();
            break;
        default:
            emit unexpectedDrawRequested();
            break;
        }
    }
    void drawDeclined() override
    {
        emit requestMove(1, 1, 2, 1);
    }
signals:
    void started();
    void drawNotClaimable();
    void unexpectedDrawRequested();
private:
    Action m_action;
};

class DrawAiPlayerFactory : public AiPlayerFactory
{
public:
    DrawAiPlayerFactory(DrawAiPlayer::Action whiteAction, DrawAiPlayer::Action blackAction) :
        m_whiteAction(whiteAction),
        m_blackAction(blackAction)
    {

    }
    AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) override
    {
        switch (colour) {
        case Chessboard::Colour::White:
            m_whiteDrawAiPlayer = new DrawAiPlayer(colour, m_whiteAction, parent);
            return m_whiteDrawAiPlayer;
        case Chessboard::Colour::Black:
        default:
            m_blackDrawAiPlayer = new DrawAiPlayer(colour, m_blackAction, parent);
            return m_blackDrawAiPlayer;
        }
    }
    DrawAiPlayer *whiteDrawAiPlayer() const { return m_whiteDrawAiPlayer; }
    DrawAiPlayer *blackDrawAiPlayer() const { return m_blackDrawAiPlayer; }
private:
    DrawAiPlayer *m_whiteDrawAiPlayer {};
    DrawAiPlayer *m_blackDrawAiPlayer {};
    DrawAiPlayer::Action m_whiteAction;
    DrawAiPlayer::Action m_blackAction;
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
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Ai;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
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
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
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
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Human;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
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
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = Chessboard::PlayerType::Ai;
        gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        gameOptions.black.playerType = Chessboard::PlayerType::Ai;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
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
    void draw_data()
    {
        QTest::addColumn<Chessboard::PlayerType>("whitePlayerType");
        QTest::addColumn<Chessboard::PlayerLocation>("whitePlayerLocation");
        QTest::addColumn<Chessboard::PlayerType>("blackPlayerType");
        QTest::addColumn<Chessboard::PlayerLocation>("blackPlayerLocation");
        QTest::addColumn<DrawAiPlayer::Action>("whiteAction");
        QTest::addColumn<DrawAiPlayer::Action>("blackAction");
        for (int i=0;i<2;++i) {
            Chessboard::PlayerType whitePlayerType = (i == 0) ? Chessboard::PlayerType::Human : Chessboard::PlayerType::Ai;
            for (int j=0;j<((whitePlayerType==Chessboard::PlayerType::Human)?2:1);++j) {
                Chessboard::PlayerLocation whitePlayerLocation = (j == 0) ? Chessboard::PlayerLocation::LocalApp : Chessboard::PlayerLocation::LocalBoard;
                for (int k=0;k<2;++k) {
                    Chessboard::PlayerType blackPlayerType = (k == 0) ? Chessboard::PlayerType::Human : Chessboard::PlayerType::Ai;
                    for (int l=0;l<((blackPlayerType==Chessboard::PlayerType::Human)?2:1);++l) {
                        Chessboard::PlayerLocation blackPlayerLocation = (l == 0) ? Chessboard::PlayerLocation::LocalApp : Chessboard::PlayerLocation::LocalBoard;
                        for (int m=0;m<3;++m) {
                            DrawAiPlayer::Action whiteAction;
                            switch (m) {
                            case 0: whiteAction = DrawAiPlayer::RequestDraw; break;
                            case 1: whiteAction = DrawAiPlayer::ClaimDraw; break;
                            case 2: whiteAction = DrawAiPlayer::AutomaticDraw; break;
                            }
                            for (int n=0;n<((whiteAction == DrawAiPlayer::RequestDraw)?2:1);++n) {
                                DrawAiPlayer::Action blackAction;
                                if (whiteAction == DrawAiPlayer::RequestDraw)
                                    blackAction = (n == 0) ? DrawAiPlayer::AcceptDraw : DrawAiPlayer::DeclineDraw;
                                else
                                    blackAction = DrawAiPlayer::NoAction;
                                QByteArray label;
                                label += (whitePlayerType == Chessboard::PlayerType::Human) ? "H" : "A";
                                label += (whitePlayerLocation == Chessboard::PlayerLocation::LocalApp) ? "P" : "B";
                                label += (blackPlayerType == Chessboard::PlayerType::Human) ? "h" : "a";
                                label += (blackPlayerLocation == Chessboard::PlayerLocation::LocalApp) ? "p" : "b";
                                switch (whiteAction) {
                                case DrawAiPlayer::RequestDraw: label += "R"; break;
                                case DrawAiPlayer::ClaimDraw: label += "C"; break;
                                case DrawAiPlayer::AutomaticDraw: default: label += "M"; break;
                                }
                                if (blackAction != DrawAiPlayer::NoAction)
                                    label += (blackAction == DrawAiPlayer::AcceptDraw) ? "t" : "d";
                                QTest::newRow(label) << whitePlayerType << whitePlayerLocation
                                                     << blackPlayerType << blackPlayerLocation
                                                     << whiteAction << blackAction;
                            }
                        }
                    }
                }
            }
        }
    }
    void APhbRt()
    {
        for (int i=0;i<100;++i) {
            DrawAiPlayerFactory drawAiPlayerFactory(DrawAiPlayer::RequestDraw, DrawAiPlayer::AcceptDraw);
            MockApplicationFacade appFacade(&drawAiPlayerFactory);
            AutoDeletePointer<MockRemoteBoard> board = new MockRemoteBoard;
            emit appFacade.connectionManager()->connected(board);
            QSignalSpy whiteStartedSpy(drawAiPlayerFactory.whiteDrawAiPlayer(), &DrawAiPlayer::started);
            QSignalSpy blackStartedSpy(drawAiPlayerFactory.blackDrawAiPlayer(), &DrawAiPlayer::started);
            QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
            QSignalSpy gameOverSpy(&appFacade, &ApplicationFacade::gameOver);
            QSignalSpy drawSpy(&appFacade, &ApplicationFacade::draw);
            QSignalSpy unexpectedDrawRequestedSpy(drawAiPlayerFactory.blackDrawAiPlayer(), &DrawAiPlayer::unexpectedDrawRequested);
            QSignalSpy drawRequestedSpy(&appFacade, &ApplicationFacade::drawRequested);
            QSignalSpy drawNotClaimableSpy(drawAiPlayerFactory.whiteDrawAiPlayer(), &DrawAiPlayer::drawNotClaimable);
            QSignalSpy drawDeclinedSpy(&appFacade, &ApplicationFacade::drawDeclined);
            QSignalSpy remoteReceivedDrawRequestedSpy(board.get(), &MockRemoteBoard::remoteReceivedDrawRequested);
            Chessboard::GameOptions gameOptions;
            gameOptions.white.playerType = Chessboard::PlayerType::Ai;
            gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
            gameOptions.black.playerType = Chessboard::PlayerType::Human;
            gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalBoard;
            appFacade.setGameOptions(gameOptions);
            QVERIFY(whiteStartedSpy.wait());
            QCOMPARE(drawRequestedSpy.count(), 0);
            if (remoteReceivedDrawRequestedSpy.isEmpty())
                QVERIFY(remoteReceivedDrawRequestedSpy.wait());
            emit board->remoteDrawRequested(Chessboard::Colour::Black);
            QCOMPARE(gameOverSpy.count(), 1);
            QCOMPARE(drawSpy.count(), 1);
            QList<QVariant> arguments = drawSpy.takeFirst();
            Chessboard::DrawReason actualReason = arguments.at(0).value<Chessboard::DrawReason>();
            Chessboard::DrawReason expectedReason = Chessboard::DrawReason::MutualAgreement;
            QCOMPARE(actualReason, expectedReason);
        }
    }
    void draw()
    {
        QFETCH(Chessboard::PlayerType, whitePlayerType);
        QFETCH(Chessboard::PlayerLocation, whitePlayerLocation);
        QFETCH(Chessboard::PlayerType, blackPlayerType);
        QFETCH(Chessboard::PlayerLocation, blackPlayerLocation);
        QFETCH(DrawAiPlayer::Action, whiteAction);
        QFETCH(DrawAiPlayer::Action, blackAction);

        DrawAiPlayerFactory drawAiPlayerFactory(whiteAction, blackAction);
        MockApplicationFacade appFacade(&drawAiPlayerFactory);
        AutoDeletePointer<MockRemoteBoard> board;
        if (whitePlayerLocation == Chessboard::PlayerLocation::LocalBoard ||
            blackPlayerLocation == Chessboard::PlayerLocation::LocalBoard) {
            board = new MockRemoteBoard;
            emit appFacade.connectionManager()->connected(board);
        }
        QSignalSpy whiteStartedSpy(drawAiPlayerFactory.whiteDrawAiPlayer(), &DrawAiPlayer::started);
        QSignalSpy blackStartedSpy(drawAiPlayerFactory.blackDrawAiPlayer(), &DrawAiPlayer::started);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        QSignalSpy gameOverSpy(&appFacade, &ApplicationFacade::gameOver);
        QSignalSpy drawSpy(&appFacade, &ApplicationFacade::draw);
        QSignalSpy unexpectedDrawRequestedSpy(drawAiPlayerFactory.blackDrawAiPlayer(), &DrawAiPlayer::unexpectedDrawRequested);
        QSignalSpy drawRequestedSpy(&appFacade, &ApplicationFacade::drawRequested);
        QSignalSpy drawNotClaimableSpy(drawAiPlayerFactory.whiteDrawAiPlayer(), &DrawAiPlayer::drawNotClaimable);
        QSignalSpy drawDeclinedSpy(&appFacade, &ApplicationFacade::drawDeclined);
        QSignalSpy remoteReceivedDrawRequestedSpy(board.get(), &MockRemoteBoard::remoteReceivedDrawRequested);
        if (whiteAction == DrawAiPlayer::ClaimDraw ||
            whiteAction == DrawAiPlayer::AutomaticDraw) {
            const QString initialState = (whiteAction == DrawAiPlayer::ClaimDraw) ?
                                             QString::fromLatin1("2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K w - b3 100 23") :
                                             QString::fromLatin1("2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K w - b3 150 23");
            appFacade.setBoardStateFromFen(initialState);
        }
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType = whitePlayerType;
        gameOptions.white.playerLocation = whitePlayerLocation;
        gameOptions.black.playerType = blackPlayerType;
        gameOptions.black.playerLocation = blackPlayerLocation;
        appFacade.setGameOptions(gameOptions);
        switch (whiteAction) {
        case DrawAiPlayer::AutomaticDraw:
            QCOMPARE(gameOverSpy.count(), 1);
            QCOMPARE(drawRequestedSpy.count(), 0);
            QCOMPARE(whiteStartedSpy.count(), 0);
            QCOMPARE(blackStartedSpy.count(), 0);
            QCOMPARE(unexpectedDrawRequestedSpy.count(), 0);
            break;
        case DrawAiPlayer::ClaimDraw:
            switch (whitePlayerType) {
            case Chessboard::PlayerType::Ai:
                QVERIFY(whiteStartedSpy.wait());
                if (gameOverSpy.isEmpty())
                    QVERIFY(gameOverSpy.wait());
                break;
            case Chessboard::PlayerType::Human:
                switch (whitePlayerLocation) {
                case Chessboard::PlayerLocation::LocalApp:
                    appFacade.requestDraw(Chessboard::Colour::White);
                    break;
                case Chessboard::PlayerLocation::LocalBoard:
                default:
                    emit board->remoteDrawRequested(Chessboard::Colour::White);
                    break;
                }
                QCOMPARE(gameOverSpy.count(), 1);
                break;
            }
            QCOMPARE(drawRequestedSpy.count(), 0);
            QCOMPARE(blackStartedSpy.count(), 0);
            QCOMPARE(drawNotClaimableSpy.count(), 0);
            QCOMPARE(unexpectedDrawRequestedSpy.count(), 0);
            break;
        default:
        case DrawAiPlayer::RequestDraw:
            switch (whitePlayerType) {
            case Chessboard::PlayerType::Ai:
                QVERIFY(whiteStartedSpy.wait());
                break;
            case Chessboard::PlayerType::Human:
                switch (whitePlayerLocation) {
                case Chessboard::PlayerLocation::LocalApp:
                    appFacade.requestDraw(Chessboard::Colour::White);
                    break;
                case Chessboard::PlayerLocation::LocalBoard:
                default:
                    emit board->remoteDrawRequested(Chessboard::Colour::White);
                    break;
                }
                break;
            }
            switch (blackPlayerType) {
            case Chessboard::PlayerType::Ai:
                switch (blackAction) {
                case DrawAiPlayer::AcceptDraw:
                    QVERIFY(gameOverSpy.wait());
                    QCOMPARE(drawRequestedSpy.count(), 0);
                    QCOMPARE(blackStartedSpy.count(), 0);
                    break;
                case DrawAiPlayer::DeclineDraw:
                default:
                    if (whitePlayerType == Chessboard::PlayerType::Human) {
                        switch (whitePlayerLocation) {
                        case Chessboard::PlayerLocation::LocalApp:
                            QVERIFY(drawDeclinedSpy.wait());
                            appFacade.requestMove(1, 1, 2, 1);
                            break;
                        case Chessboard::PlayerLocation::LocalBoard:
                        default:
                            emit board->remoteMove(1, 1, 2, 1);
                            break;
                        }
                    }
                    QVERIFY(blackStartedSpy.wait());
                    QCOMPARE(drawRequestedSpy.count(), 0);
                    break;
                }
                break;
            case Chessboard::PlayerType::Human:
                switch (blackAction) {
                case DrawAiPlayer::AcceptDraw:
                    switch (blackPlayerLocation) {
                    case Chessboard::PlayerLocation::LocalApp:
                        if (drawRequestedSpy.isEmpty())
                            QVERIFY(drawRequestedSpy.wait());
                        QCOMPARE(drawRequestedSpy.count(), 1);
                        appFacade.requestDraw(Chessboard::Colour::Black);
                        break;
                    case Chessboard::PlayerLocation::LocalBoard:
                    default:
                        QCOMPARE(drawRequestedSpy.count(), 0);
                        if (remoteReceivedDrawRequestedSpy.isEmpty())
                            QVERIFY(remoteReceivedDrawRequestedSpy.wait());
                        emit board->remoteDrawRequested(Chessboard::Colour::Black);
                        break;
                    }
                    QCOMPARE(gameOverSpy.count(), 1);
                    break;
                case DrawAiPlayer::DeclineDraw:
                default:
                    switch (blackPlayerLocation) {
                    case Chessboard::PlayerLocation::LocalApp:
                        if (drawRequestedSpy.isEmpty())
                            QVERIFY(drawRequestedSpy.wait());
                        QCOMPARE(drawRequestedSpy.count(), 1);
                        appFacade.declineDraw(Chessboard::Colour::Black);
                        break;
                    case Chessboard::PlayerLocation::LocalBoard:
                    default:
                        QCOMPARE(drawRequestedSpy.count(), 0);
                        emit board->remoteDrawDeclined(Chessboard::Colour::Black);
                        break;
                    }
                    break;
                }
                break;
            }
            break;
        }
        if (blackAction != DrawAiPlayer::DeclineDraw) {
            QCOMPARE(drawSpy.count(), 1);
            QList<QVariant> arguments = drawSpy.takeFirst();
            Chessboard::DrawReason actualReason = arguments.at(0).value<Chessboard::DrawReason>();
            Chessboard::DrawReason expectedReason;
            switch (whiteAction) {
            case DrawAiPlayer::ClaimDraw: expectedReason = Chessboard::DrawReason::FiftyMoveRule; break;
            case DrawAiPlayer::AutomaticDraw: expectedReason = Chessboard::DrawReason::SeventyFiveMoveRule; break;
            case DrawAiPlayer::RequestDraw: default: expectedReason = Chessboard::DrawReason::MutualAgreement; break;
            }
            QCOMPARE(actualReason, expectedReason);
        } else if (whitePlayerType == Chessboard::PlayerType::Human &&
                   whitePlayerLocation == Chessboard::PlayerLocation::LocalApp) {
            QCOMPARE(drawDeclinedSpy.count(), 1);
        } else {
            QCOMPARE(drawDeclinedSpy.count(), 0);
        }
    }
};

QTEST_MAIN(TestApplicationFacade)

#include "tst_applicationfacade.moc"
