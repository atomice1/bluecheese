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

#include <QTest>

#include "chessboard.h"

using namespace Chessboard;

class TestBoardState : public QObject
{
    Q_OBJECT
private slots:
    void fenRecordRoundTrip()
    {
        BoardState state = BoardState::newGame();
        const QString record = state.toFenString();
        const BoardState state2 = BoardState::fromFenString(record);
        const QString record2 = state2.toFenString();
        QCOMPARE(record2, record);
    }

    void initialBoardFenRecord()
    {
        BoardState state = BoardState::newGame();
        const QString expected = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        const QString actual = state.toFenString();
        QCOMPARE(actual, expected);
    }

    void fenRecordAfterMoves()
    {
        BoardState state = BoardState::newGame();
        state.move(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("e4"));
        const QString expected1 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        const QString actual1 = state.toFenString();
        QCOMPARE(actual1, expected1);
        state.move(Square::fromAlgebraicString("c7"), Square::fromAlgebraicString("c5"));
        const QString expected2 = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
        const QString actual2 = state.toFenString();
        QCOMPARE(actual2, expected2);
        state.move(Square::fromAlgebraicString("g1"), Square::fromAlgebraicString("f3"));
        const QString expected3 = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
        const QString actual3 = state.toFenString();
        QCOMPARE(actual3, expected3);
    }

    void castlingRights()
    {
        const QString whiteInitialState = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
        BoardState state = BoardState::fromFenString(whiteInitialState);
        QVERIFY(state.isValid());
        const QString actual1 = state.toFenString();
        QCOMPARE(actual1, whiteInitialState);
        // Move white kingside rook; check white kingside castling is no longer available.
        state.move(Square::fromAlgebraicString("h1"), Square::fromAlgebraicString("g1"));
        const QString expected2 = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K1R1 b Qkq - 1 1";
        const QString actual2 = state.toFenString();
        QCOMPARE(actual2, expected2);
        // Move white queenside rook; check white queenside castling is no longer available.
        state = BoardState::fromFenString(whiteInitialState);
        state.move(Square::fromAlgebraicString("a1"), Square::fromAlgebraicString("b1"));
        const QString expected3 = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/1R2K2R b Kkq - 1 1";
        const QString actual3 = state.toFenString();
        QCOMPARE(actual3, expected3);
        // Move white king; check both white castlings are no longer vailable.
        state = BoardState::fromFenString(whiteInitialState);
        state.move(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("d1"));
        const QString expected4 = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R2K3R b kq - 1 1";
        const QString actual4 = state.toFenString();
        QCOMPARE(actual4, expected4);

        const QString blackInitialState = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1";
        state = BoardState::fromFenString(blackInitialState);
        QVERIFY(state.isValid());
        const QString actual5 = state.toFenString();
        QCOMPARE(actual5, blackInitialState);
        // Move black kingside rook; check black kingside castling is no longer available.
        state.move(Square::fromAlgebraicString("h8"), Square::fromAlgebraicString("g8"));
        const QString expected6 = "r3k1r1/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQq - 1 2";
        const QString actual6 = state.toFenString();
        QCOMPARE(actual6, expected6);
        // Move black queenside rook; check black queenside castling is no longer available.
        state = BoardState::fromFenString(blackInitialState);
        state.move(Square::fromAlgebraicString("a8"), Square::fromAlgebraicString("b8"));
        const QString expected7 = "1r2k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQk - 1 2";
        const QString actual7 = state.toFenString();
        QCOMPARE(actual7, expected7);
        // Move black king; check both black castlings are no longer vailable.
        state = BoardState::fromFenString(blackInitialState);
        state.move(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("d8"));
        const QString expected8 = "r2k3r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 1 2";
        const QString actual8 = state.toFenString();
        QCOMPARE(actual8, expected8);
    }

    void legalMovePawn()
    {
        BoardState state = BoardState::newGame();
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("e4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("e3")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("f6")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("f5")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("e5")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("f3")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("d3")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("a1")));
        state.move(Square::fromAlgebraicString("e2"), Square::fromAlgebraicString("e4"));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("f6")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("f5")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("e3")));
        state.move(Square::fromAlgebraicString("f7"), Square::fromAlgebraicString("f5"));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e4"), Square::fromAlgebraicString("e5")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e4"), Square::fromAlgebraicString("f5")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e4"), Square::fromAlgebraicString("d5")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e4"), Square::fromAlgebraicString("e6")));
        state.move(Square::fromAlgebraicString("e4"), Square::fromAlgebraicString("f5"));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("g7"), Square::fromAlgebraicString("g5")));
        state.move(Square::fromAlgebraicString("g7"), Square::fromAlgebraicString("g5"));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("f5"), Square::fromAlgebraicString("f6")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("f5"), Square::fromAlgebraicString("g6"))); // en passant capture
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("f5"), Square::fromAlgebraicString("e6")));
        state.move(Square::fromAlgebraicString("f5"), Square::fromAlgebraicString("g6"));
        const QString expected = "rnbqkbnr/ppppp2p/6P1/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3";
        const QString actual = state.toFenString();
        QCOMPARE(actual, expected);
        state.move(Square::fromAlgebraicString("a2"), Square::fromAlgebraicString("a4"));
        state.move(Square::fromAlgebraicString("a7"), Square::fromAlgebraicString("a5"));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("a4"), Square::fromAlgebraicString("a5")));
    }

    void legalMoveQueen()
    {
        BoardState state = BoardState::newGame();
        state[Square::fromAlgebraicString("d1")] = ColouredPiece::None;
        state[Square::fromAlgebraicString("d4")] = ColouredPiece(Colour::White, Piece::Queen);

        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d3")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d5")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d6")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("a4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("b4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("c4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("e4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("f4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("g4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("h4")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("c3")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("c5")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("b6")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("e5")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("f6")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("e3")));

        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d7")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("a7")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("g7")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("h8")));

        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d2")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("d1")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("b2")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("a1")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("f2")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("d4"), Square::fromAlgebraicString("g1")));
    }

    void checkmate()
    {
        BoardState state = BoardState::newGame();
        QVERIFY(!state.isCheckmate());
        state = BoardState::fromFenString("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
        QVERIFY(state.isValid());
        QVERIFY(!state.isCheck());
        QVERIFY(!state.isCheckmate());
        state = BoardState::fromFenString("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
        QVERIFY(state.isValid());
        QVERIFY(!state.isCheck());
        QVERIFY(!state.isCheckmate());
        state = BoardState::fromFenString("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
        QVERIFY(state.isValid());
        QVERIFY(!state.isCheck());
        QVERIFY(!state.isCheckmate());

        state = BoardState::fromFenString("3b1q1q/1N2PRQ1/rR3KBr/B4PP1/2Pk1r1b/1P2P1N1/2P2P2/8 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("R5k1/5ppp/8/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("8/4Nppk/8/7R/8/8/8/6K1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6kR/6P1/5K2/8/8/8/8/8 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/7R/5N2/8/8/8/8/6K1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("5rk1/7B/8/4B1N1/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("2kr4/3n4/B7/4B3/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("5qk1/6pQ/6P1/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/5N1p/8/6R1/8/8/8/6K1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("8/8/8/8/6p1/5qk1/7Q/6K1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6k1/6Q1/5B2/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("8/8/6qp/6pk/6P1/5P2/5B2/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/7p/8/3B4/3B4/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6q1/6kp/5Q2/6P1/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("3rkr2/8/4Q3/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/6p1/8/7Q/2B5/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("4R3/4kp2/5N2/4P3/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6kR/5p2/5B2/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("3q1b2/4kB2/3p4/3NN3/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6k1/5pQ1/5Pp1/8/8/8/8/6K1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6Q1/5Bpk/7p/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6k1/6pp/6N1/3B4/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/7p/8/4B1R1/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("1nbB4/1pk5/2p5/8/8/8/8/3RK3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("3Rk3/5p2/5B2/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("5rk1/5p1p/8/8/8/8/1B6/4K1R1 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6k1/6Q1/5P2/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("6rk/5Npp/8/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("5rk1/4Np1p/5B2/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("3r1r2/4k3/4Q3/3B4/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("7k/8/8/6R1/7Q/8/8/7K b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
        state = BoardState::fromFenString("5Q1k/7p/8/8/8/8/8/4K3 b - -");
        QVERIFY(state.isValid());
        QVERIFY(state.isCheckmate());
    }

    void check()
    {
        BoardState state = BoardState::newGame();
        QVERIFY(!state.isCheckmate());
        QVERIFY(!state.isCheck());
        state = BoardState::fromFenString("qrb5/rk1p1K2/p2P4/Pp6/1N2B3/6p1/5n2/6b1 b - - 0 1");
        QVERIFY(!state.isCheckmate());
        QVERIFY(state.isCheck());
    }

    void canCastle()
    {
        const QString whiteInitialState = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
        BoardState state = BoardState::fromFenString(whiteInitialState);
        QVERIFY(state.isValid());
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("h1")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("b1")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("a1")));
        state.activeColour = Colour::Black;
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("h8")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("b8")));
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("a8")));
        state.activeColour = Colour::White;
        state.whiteKingsideCastlingAvailable = false;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        state = BoardState::fromFenString(whiteInitialState);
        state.whiteQueensideCastlingAvailable = false;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state.blackKingsideCastlingAvailable = false;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        QVERIFY(state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state.blackQueensideCastlingAvailable = false;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        state = BoardState::fromFenString(whiteInitialState);
        state[Square::fromAlgebraicString("g2")] = ColouredPiece::None;
        state[Square::fromAlgebraicString("g4")] = ColouredPiece(Colour::Black, Piece::Rook);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        state = BoardState::fromFenString(whiteInitialState);
        state[Square::fromAlgebraicString("d2")] = ColouredPiece::None;
        state[Square::fromAlgebraicString("d4")] = ColouredPiece(Colour::Black, Piece::Rook);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state[Square::fromAlgebraicString("g7")] = ColouredPiece::None;
        state[Square::fromAlgebraicString("g5")] = ColouredPiece(Colour::White, Piece::Rook);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state[Square::fromAlgebraicString("d7")] = ColouredPiece::None;
        state[Square::fromAlgebraicString("d5")] = ColouredPiece(Colour::White, Piece::Rook);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::White;
        state[Square::fromAlgebraicString("g1")] = ColouredPiece(Colour::White, Piece::Bishop);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        state[Square::fromAlgebraicString("b1")] = ColouredPiece(Colour::White, Piece::Bishop);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        state.activeColour = Colour::Black;
        state[Square::fromAlgebraicString("g7")] = ColouredPiece(Colour::White, Piece::Bishop);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        state[Square::fromAlgebraicString("b7")] = ColouredPiece(Colour::White, Piece::Bishop);
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        // missing rook
        state = BoardState::fromFenString(whiteInitialState);
        state[Square::fromAlgebraicString("h1")] = ColouredPiece::None;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        state = BoardState::fromFenString(whiteInitialState);
        state[Square::fromAlgebraicString("a1")] = ColouredPiece::None;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state[Square::fromAlgebraicString("h8")] = ColouredPiece::None;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        state[Square::fromAlgebraicString("a8")] = ColouredPiece::None;
        QVERIFY(!state.isLegalMove(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
    }

    void castling()
    {
        const QString whiteInitialState = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
        BoardState state = BoardState::fromFenString(whiteInitialState);
        QVERIFY(state.isValid());
        QVERIFY(state.move(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("g1")));
        QVERIFY(state[Square::fromAlgebraicString("g1")] == ColouredPiece(Colour::White, Piece::King));
        QVERIFY(state[Square::fromAlgebraicString("f1")] == ColouredPiece(Colour::White, Piece::Rook));
        state = BoardState::fromFenString(whiteInitialState);
        QVERIFY(state.move(Square::fromAlgebraicString("e1"), Square::fromAlgebraicString("c1")));
        QVERIFY(state[Square::fromAlgebraicString("c1")] == ColouredPiece(Colour::White, Piece::King));
        QVERIFY(state[Square::fromAlgebraicString("d1")] == ColouredPiece(Colour::White, Piece::Rook));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        QVERIFY(state.move(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("g8")));
        QVERIFY(state[Square::fromAlgebraicString("g8")] == ColouredPiece(Colour::Black, Piece::King));
        QVERIFY(state[Square::fromAlgebraicString("f8")] == ColouredPiece(Colour::Black, Piece::Rook));
        state = BoardState::fromFenString(whiteInitialState);
        state.activeColour = Colour::Black;
        QVERIFY(state.move(Square::fromAlgebraicString("e8"), Square::fromAlgebraicString("c8")));
        QVERIFY(state[Square::fromAlgebraicString("c8")] == ColouredPiece(Colour::Black, Piece::King));
        QVERIFY(state[Square::fromAlgebraicString("d8")] == ColouredPiece(Colour::Black, Piece::Rook));
    }

    void promotion()
    {
        const QString whiteInitialState = "4k3/Pppppppp/8/8/8/8/1PPPPPPP/4K3 w KQkq - 0 1";
        BoardState state = BoardState::fromFenString(whiteInitialState);
        bool promotion = false;
        QVERIFY(state.move(Square::fromAlgebraicString("h2"), Square::fromAlgebraicString("h4"), &promotion));
        QVERIFY(!promotion);
        state = BoardState::fromFenString(whiteInitialState);
        promotion = false;
        QVERIFY(state.move(Square::fromAlgebraicString("a7"), Square::fromAlgebraicString("a8"), &promotion));
        QVERIFY(promotion);
        QVERIFY(state.activeColour == Colour::White);
        QVERIFY(state.promote(Piece::Queen));
        QVERIFY(state[Square::fromAlgebraicString("a8")] == ColouredPiece(Colour::White, Piece::Queen));
        QVERIFY(state.activeColour == Colour::Black);

        const QString blackInitialState = "4k3/ppppppp1/8/8/8/8/PPPPPPPp/4K3 b KQkq - 0 1";
        state = BoardState::fromFenString(blackInitialState);
        promotion = false;
        QVERIFY(state.move(Square::fromAlgebraicString("a7"), Square::fromAlgebraicString("a5"), &promotion));
        QVERIFY(!promotion);
        state = BoardState::fromFenString(blackInitialState);
        promotion = false;
        QVERIFY(state.move(Square::fromAlgebraicString("h2"), Square::fromAlgebraicString("h1"), &promotion));
        QVERIFY(promotion);
        QVERIFY(state.activeColour == Colour::Black);
        QVERIFY(state.promote(Piece::Queen));
        QVERIFY(state[Square::fromAlgebraicString("h1")] == ColouredPiece(Colour::Black, Piece::Queen));
        QVERIFY(state.activeColour == Colour::White);
    }

    void isLegal()
    {
        BoardState state = BoardState::newGame();
        IllegalBoardReason reason;
        bool legal = state.isLegal(&reason);
        QVERIFY(legal);
    }
};

QTEST_MAIN(TestBoardState)

#include "tst_boardstate.moc"
