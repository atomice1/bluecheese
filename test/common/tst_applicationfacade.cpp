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
public:
    BlackPromotionAiPlayer(Chessboard::Colour colour, QObject *parent) :
        AiPlayer(colour, parent) {}
    void start(const Chessboard::BoardState& state) override
    {
        qDebug("BlackPromotionAiPlayer::start");
        emit error(UnknownError);
    }
    void promotionRequired() override
    {
    }
    void drawRequested() override
    {
    }
private:
    bool *m_startSentinel;
};

class PromotionAiPlayerFactory : public AiPlayerFactory
{
public:
    AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr) override
    {
        qDebug("Creating a PromotionAiPlayer");
        switch (colour) {
        case Chessboard::Colour::White:
            return new WhitePromotionAiPlayer(colour, parent);
        case Chessboard::Colour::Black:
            return new BlackPromotionAiPlayer(colour, parent);
        }
    }
private:
    bool *m_blackStartSentinel;
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
        gameOptions.black.playerType = Chessboard::PlayerType::Human;
        gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        appFacade.setGameOptions(gameOptions);
        QSignalSpy activeColourChangedSpy(&appFacade, &ApplicationFacade::activeColourChanged);
        QSignalSpy boardStateChangedSpy(&appFacade, &ApplicationFacade::boardStateChanged);
        const QString initialState = "3rk3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        appFacade.setBoardStateFromFen(initialState);
        bool waitOk = false;
        bool seenPromotion = false;
        Chessboard::Colour colour = Chessboard::Colour::White;
        do {
            if (colour == Chessboard::Colour::Black) {
                waitOk = boardStateChangedSpy.wait();
            } else {
                waitOk = activeColourChangedSpy.wait();
            }
            if (!waitOk)
                break;
            while (!activeColourChangedSpy.isEmpty()) {
                QList<QVariant> arguments = activeColourChangedSpy.takeFirst();
                colour = arguments.at(0).value<Chessboard::Colour>();
            }
            while (!boardStateChangedSpy.isEmpty()) {
                QList<QVariant> arguments = boardStateChangedSpy.takeFirst();
                BoardState state = arguments.at(0).value<Chessboard::BoardState>();
                if (state.activeColour == Colour::White) {
                    QVERIFY(state[Square::fromAlgebraicString("a8")] != ColouredPiece(Colour::White, Piece::Rook));
                } else {
                    QCOMPARE(state[Square::fromAlgebraicString("a8")], ColouredPiece(Colour::White, Piece::Rook));
                    seenPromotion = true;
                }
            }
        } while (colour != Chessboard::Colour::Black || !seenPromotion);
        QVERIFY(waitOk);
    }
};

QTEST_MAIN(TestApplicationFacade)

#include "tst_applicationfacade.moc"
