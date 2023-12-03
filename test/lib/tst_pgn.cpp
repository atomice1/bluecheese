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

class TestPgn : public QObject
{
    Q_OBJECT
private slots:
    void parseString()
    {
        PgnParser parser;
        QString errorMessage;
        Pgn pgn = parser.parse(QLatin1String(
"[Event \"F/S Return Match\"]\n"
"[Site \"Belgrade, Serbia JUG\"]\n"
"[Date \"1992.11.04\"]\n"
"[Round \"29\"]\n"
"[White \"Fischer, Robert J.\"]\n"
"[Black \"Spassky, Boris V.\"]\n"
"[Result \"1/2-1/2\"]\n"
"\n"
"1. e4 e5 2. Nf3 Nc6 3. Bb5 {This opening is called the Ruy Lopez.} 3... a6\n"
"4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 d6 8. c3 O-O 9. h3 Nb8 10. d4 Nbd7\n"
"11. c4 c6 12. cxb5 axb5 13. Nc3 Bb7 14. Bg5 b4 15. Nb1 h6 16. Bh4 c5 17. dxe5\n"
"Nxe4 18. Bxe7 Qxe7 19. exd6 Qf6 20. Nbd2 Nxd6 21. Nc4 Nxc4 22. Bxc4 Nb6\n"
"23. Ne5 Rae8 24. Bxf7+ Rxf7 25. Nxf7 Rxe1+ 26. Qxe1 Kxf7 27. Qe3 Qg5 28. Qxg5\n"
"hxg5 29. b3 Ke6 30. a3 Kd6 31. axb4 cxb4 32. Ra5 Nd5 33. f3 Bc8 34. Kf2 Bf5\n"
"35. Ra7 g6 36. Ra6+ Kc5 37. Ke1 Nf4 38. g3 Nxh3 39. Kd2 Kb5 40. Rd6 Kc5 41. Ra6\n"
"Nf2 42. g4 Bd3 43. Re6 1/2-1/2"
        ), &errorMessage);
        QCOMPARE(errorMessage, QString());
        QVERIFY(pgn.isValid());
        QVERIFY(pgn.moves.size() == 85);
        QVERIFY(pgn.tags.value(QLatin1String("White")) == QLatin1String("Fischer, Robert J."));
        pgn = parser.parse(QLatin1String(
"[Event \"Third Rosenwald Trophy\"]\n"
"[Site \"New York, NY USA\"]\n"
"[Date \"1956.10.17\"]\n"
"[EventDate \"1956.10.07\"]\n"
"[Round \"8\"]\n"
"[Result \"0-1\"]\n"
"[White \"Donald Byrne\"]\n"
"[Black \"Robert James Fischer\"]\n"
"[ECO \"D92\"]\n"
"[WhiteElo \"?\"]\n"
"[BlackElo \"?\"]\n"
"[PlyCount \"82\"]\n"
"\n"
"1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3 dxc4 7. Qxc4 c6 8. e4 Nbd7 9. Rd1 Nb6 10. Qc5 Bg4 11. Bg5 Na4 12. Qa3 Nxc3 13. bxc3 Nxe4 14. Bxe7 Qb6 15. Bc4 Nxc3 16. Bc5 Rfe8+ 17. Kf1 Be6 18. Bxb6 Bxc4+ 19. Kg1 Ne2+ 20. Kf1 Nxd4+ 21. Kg1 Ne2+ 22. Kf1 Nc3+ 23. Kg1 axb6 24. Qb4 Ra4 25. Qxb6 Nxd1 26. h3 Rxa2 27. Kh2 Nxf2 28. Re1 Rxe1 29. Qd8+ Bf8 30. Nxe1 Bd5 31. Nf3 Ne4 32. Qb8 b5 33. h4 h5 34. Ne5 Kg7 35. Kg1 Bc5+ 36. Kf1 Ng3+ 37. Ke1 Bb4+ 38. Kd1 Bb3+ 39. Kc1 Ne2+ 40. Kb1 Nc3+ 41. Kc1 Rc2#\n"
        ), &errorMessage);
        QCOMPARE(errorMessage, QString());
        QVERIFY(pgn.isValid());
        QCOMPARE(pgn.tags.value(QLatin1String("Site")), QLatin1String("New York, NY USA"));
        QCOMPARE(pgn.moves.size(), 82);
        BoardState state = BoardState::newGame();
        for (const AlgebraicNotation& an : pgn.moves) {
            QVERIFY(state.move(an));
        }
        QVERIFY(state.isCheckmate());
    }
};

QTEST_MAIN(TestPgn)

#include "tst_pgn.moc"
