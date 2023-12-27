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

class TestAlgebraicNotation : public QObject
{
    Q_OBJECT
private slots:
    void pieceNames()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("Nc6");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Knight);
        QCOMPARE(an.toCol, 2);
        QCOMPARE(an.toRow, 5);
        an = AlgebraicNotation::fromString("Be5");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Bishop);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 4);
        an = AlgebraicNotation::fromString("Rd1");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Rook);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.toRow, 0);
        an = AlgebraicNotation::fromString("Qb3");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Queen);
        QCOMPARE(an.toCol, 1);
        QCOMPARE(an.toRow, 2);
        an = AlgebraicNotation::fromString("c4");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 2);
        QCOMPARE(an.toRow, 3);
    }
    void captures()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("Bxe5");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Bishop);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 4);
        QVERIFY(an.capture);
        an = AlgebraicNotation::fromString("exd5");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.toRow, 4);
        QCOMPARE(an.fromCol, 4);
        QVERIFY(an.capture);
        an = AlgebraicNotation::fromString("exd6 e.p.");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.toRow, 5);
        QCOMPARE(an.fromCol, 4);
        QVERIFY(an.enPassant);
        an = AlgebraicNotation::fromString("e:d5");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.toRow, 4);
        QCOMPARE(an.fromCol, 4);
        QVERIFY(an.capture);
        an = AlgebraicNotation::fromString("ed5:");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.toRow, 4);
        QCOMPARE(an.fromCol, 4);
        QVERIFY(an.capture);
        an = AlgebraicNotation::fromString("exd");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.fromCol, 4);
        QVERIFY(an.capture);
        an = AlgebraicNotation::fromString("ed");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 3);
        QCOMPARE(an.fromCol, 4);
        an = AlgebraicNotation::fromString("Qh4xe1");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Queen);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 0);
        QCOMPARE(an.fromCol, 7);
        QCOMPARE(an.fromRow, 3);
    }
    void moves()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("Rdf8");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Rook);
        QCOMPARE(an.toCol, 5);
        QCOMPARE(an.toRow, 7);
        QCOMPARE(an.fromCol, 3);
        an = AlgebraicNotation::fromString("R1a3");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Rook);
        QCOMPARE(an.toCol, 0);
        QCOMPARE(an.toRow, 2);
        QCOMPARE(an.fromRow, 0);
        an = AlgebraicNotation::fromString("Qh4e1");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Queen);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 0);
        QCOMPARE(an.fromCol, 7);
        QCOMPARE(an.fromRow, 3);
    }
    void promotion()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("e8Q");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 7);
        QVERIFY(an.promotion);
        QCOMPARE(an.promotionPiece, Piece::Queen);
        an = AlgebraicNotation::fromString("e8=Q");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 7);
        QVERIFY(an.promotion);
        QCOMPARE(an.promotionPiece, Piece::Queen);
        an = AlgebraicNotation::fromString("e8(Q)");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 7);
        QVERIFY(an.promotion);
        QCOMPARE(an.promotionPiece, Piece::Queen);
        an = AlgebraicNotation::fromString("e8/Q");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Pawn);
        QCOMPARE(an.toCol, 4);
        QCOMPARE(an.toRow, 7);
        QVERIFY(an.promotion);
        QCOMPARE(an.promotionPiece, Piece::Queen);
        an = AlgebraicNotation::fromString("b7b8r");
        QVERIFY(an.isValid());
        QCOMPARE(an.fromCol, 1);
        QCOMPARE(an.fromRow, 6);
        QCOMPARE(an.toCol, 1);
        QCOMPARE(an.toRow, 7);
        QVERIFY(an.promotion);
        QCOMPARE(an.promotionPiece, Piece::Rook);
    }
    void draw()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("Nc6(=)");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::Knight);
        QCOMPARE(an.toCol, 2);
        QCOMPARE(an.toRow, 5);
        QVERIFY(an.drawOffered);
    }
    void castling()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("O-O");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::King);
        QCOMPARE(an.castling, Castling::KingsideCastling);
        an = AlgebraicNotation::fromString("0-0");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::King);
        QCOMPARE(an.castling, Castling::KingsideCastling);
        an = AlgebraicNotation::fromString("O-O-O");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::King);
        QCOMPARE(an.castling, Castling::QueensideCastling);
        an = AlgebraicNotation::fromString("0-0-0");
        QVERIFY(an.isValid());
        QCOMPARE(an.piece, Piece::King);
        QCOMPARE(an.castling, Castling::QueensideCastling);
    }
    void checkStatus()
    {
        AlgebraicNotation an = AlgebraicNotation::fromString("Ng3+");
        QVERIFY(an.isValid());
        QCOMPARE(an.checkStatus, CheckStatus::Check);
        an = AlgebraicNotation::fromString("Ng3#");
        QVERIFY(an.isValid());
        QCOMPARE(an.checkStatus, CheckStatus::Checkmate);
        an = AlgebraicNotation::fromString("Ng3++");
        QVERIFY(an.isValid());
        QCOMPARE(an.checkStatus, CheckStatus::Checkmate);
    }
};

QTEST_MAIN(TestAlgebraicNotation)

#include "tst_algebraicnotation.moc"
