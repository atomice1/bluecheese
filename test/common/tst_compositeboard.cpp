/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
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

#include "compositeboard.h"
#include "chessboard.h"

using namespace Chessboard;

class MockRemoteBoard : public RemoteBoard
{
public:
    MockRemoteBoard(QObject *parent = nullptr) :
        RemoteBoard(BoardAddress(), parent) {}
};

class TestCompositeBoard : public QObject
{
    Q_OBJECT
private slots:
    void promotionLocal()
    {
        CompositeBoard board;
        QSignalSpy promotionSpy(&board, &CompositeBoard::promotionRequired);
        const QString initialState = "4k3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        board.setBoardState(BoardState::fromFenString(initialState));
        board.requestMove(Square::fromAlgebraicString("a7"), Square::fromAlgebraicString("a8"));
        QCOMPARE(promotionSpy.count(), 1);
        QSignalSpy checkmateSpy(&board, &CompositeBoard::checkmate);
        board.requestPromotion(Piece::Queen);
        const QString finalState = "Q3k3/1ppppppp/8/8/8/8/1PPPPPPP/4K3 b KQkq - 0 1";
        QCOMPARE(board.boardState().toFenString(), finalState);
        QCOMPARE(checkmateSpy.count(), 1);
    }

    void mutualDrawBothLocal()
    {
        CompositeBoard board;
        QSignalSpy drawRequestedSpy(&board, &CompositeBoard::drawRequested);
        QSignalSpy drawSpy(&board, &CompositeBoard::draw);
        board.requestDraw(Colour::Black);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 0);
        board.requestDraw(Colour::White);
        QCOMPARE(drawSpy.count(), 1);
    }
    void mutualDrawBothRemote()
    {
        CompositeBoard board;
        MockRemoteBoard remoteBoard;
        board.setRemoteBoard(&remoteBoard);
        QSignalSpy drawRequestedSpy(&board, &CompositeBoard::drawRequested);
        QSignalSpy drawSpy(&board, &CompositeBoard::draw);
        emit remoteBoard.remoteDrawRequested(Colour::Black);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 0);
        emit remoteBoard.remoteDrawRequested(Colour::White);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 1);
    }
    void mutualDrawLocalRemote()
    {
        CompositeBoard board;
        MockRemoteBoard remoteBoard;
        board.setRemoteBoard(&remoteBoard);
        QSignalSpy drawRequestedSpy(&board, &CompositeBoard::drawRequested);
        QSignalSpy drawSpy(&board, &CompositeBoard::draw);
        board.requestDraw(Colour::Black);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 0);
        emit remoteBoard.remoteDrawRequested(Colour::White);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 1);
    }
    void mutualDrawRemoteLocal()
    {
        CompositeBoard board;
        MockRemoteBoard remoteBoard;
        board.setRemoteBoard(&remoteBoard);
        QSignalSpy drawRequestedSpy(&board, &CompositeBoard::drawRequested);
        QSignalSpy drawSpy(&board, &CompositeBoard::draw);
        emit remoteBoard.remoteDrawRequested(Colour::Black);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 0);
        board.requestDraw(Colour::White);
        QCOMPARE(drawRequestedSpy.count(), 1);
        QCOMPARE(drawSpy.count(), 1);
    }
};

QTEST_MAIN(TestCompositeBoard)

#include "tst_compositeboard.moc"
