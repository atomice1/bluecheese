/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "chessboard.h"

namespace Chessboard {

QString ColouredPiece::toFenString() const
{
    switch (m_value) {
    case ColouredPiece::None:
        return " ";
    case ColouredPiece::WhitePawn:
        return "P";
    case ColouredPiece::WhiteRook:
        return "R";
    case ColouredPiece::WhiteKnight:
        return "N";
    case ColouredPiece::WhiteBishop:
        return "B";
    case ColouredPiece::WhiteQueen:
        return "Q";
    case ColouredPiece::WhiteKing:
        return "K";
    case ColouredPiece::BlackPawn:
        return "p";
    case ColouredPiece::BlackRook:
        return "r";
    case ColouredPiece::BlackKnight:
        return "n";
    case ColouredPiece::BlackBishop:
        return "b";
    case ColouredPiece::BlackQueen:
        return "q";
    case ColouredPiece::BlackKing:
        return "k";
    default:
        return "?";
    }
}

ColouredPiece ColouredPiece::fromFenString(const QString& fen)
{
    if (fen.length() != 1)
        return ColouredPiece();
    return fromFenChar(fen.at(0));
}

ColouredPiece ColouredPiece::fromFenChar(QChar fen)
{
    switch (fen.toLatin1()) {
    case ' ':
        return None;
    case 'P':
        return WhitePawn;
    case 'R':
        return WhiteRook;
    case 'N':
        return WhiteKnight;
    case 'B':
        return WhiteBishop;
    case 'Q':
        return WhiteQueen;
    case 'K':
        return WhiteKing;
    case 'p':
        return BlackPawn;
    case 'r':
        return BlackRook;
    case 'n':
        return BlackKnight;
    case 'b':
        return BlackBishop;
    case 'q':
        return BlackQueen;
    case 'k':
        return BlackKing;
    default:
        return ColouredPiece();
    }
}

QString ColouredPiece::toUnicode() const
{
    switch (m_value) {
    case ColouredPiece::None:
        return " ";
    case ColouredPiece::WhitePawn:
        return QChar(0x2659);
    case ColouredPiece::WhiteRook:
        return QChar(0x2656);
    case ColouredPiece::WhiteKnight:
        return QChar(0x2658);
    case ColouredPiece::WhiteBishop:
        return QChar(0x2657);
    case ColouredPiece::WhiteQueen:
        return QChar(0x2655);
    case ColouredPiece::WhiteKing:
        return QChar(0x2654);
    case ColouredPiece::BlackPawn:
        return QChar(0x265F);
    case ColouredPiece::BlackRook:
        return QChar(0x265C);
    case ColouredPiece::BlackKnight:
        return QChar(0x265E);
    case ColouredPiece::BlackBishop:
        return QChar(0x265D);
    case ColouredPiece::BlackQueen:
        return QChar(0x265B);
    case ColouredPiece::BlackKing:
        return QChar(0x265A);
    default:
        return "?";
    }
}

QString Square::toString() const
{
    return toAlgebraicString();
}

QString Square::toAlgebraicString() const
{
    return QChar('a' + col) + QString::number(row + 1);
}

Square Square::fromAlgebraicString(const QString& s)
{
    if (s.length() != 2)
        return Square();
    int col = s.at(0).toLatin1() - 'a';
    if (col < 0 || col >= 8)
        return Square();
    int row = s.at(1).toLatin1() - '1';
    if (row < 0 || row >= 8)
        return Square();
    return Square(row, col);
}

BoardState::BoardState() :
    fullMoveCount(-1)
{
}

BoardState BoardState::newGame()
{
    BoardState ret;
    ret.state[0][0] = ColouredPiece::WhiteRook;
    ret.state[0][1] = ColouredPiece::WhiteKnight;
    ret.state[0][2] = ColouredPiece::WhiteBishop;
    ret.state[0][3] = ColouredPiece::WhiteQueen;
    ret.state[0][4] = ColouredPiece::WhiteKing;
    ret.state[0][5] = ColouredPiece::WhiteBishop;
    ret.state[0][6] = ColouredPiece::WhiteKnight;
    ret.state[0][7] = ColouredPiece::WhiteRook;
    ret.state[7][0] = ColouredPiece::BlackRook;
    ret.state[7][1] = ColouredPiece::BlackKnight;
    ret.state[7][2] = ColouredPiece::BlackBishop;
    ret.state[7][3] = ColouredPiece::BlackQueen;
    ret.state[7][4] = ColouredPiece::BlackKing;
    ret.state[7][5] = ColouredPiece::BlackBishop;
    ret.state[7][6] = ColouredPiece::BlackKnight;
    ret.state[7][7] = ColouredPiece::BlackRook;
    for (int i=0;i<8;++i) {
        ret.state[1][i] = ColouredPiece::WhitePawn;
        ret.state[6][i] = ColouredPiece::BlackPawn;
    }
    ret.activeColour = Colour::White;
    ret.whiteKingsideCastlingAvailable = true;
    ret.whiteQueensideCastlingAvailable = true;
    ret.blackKingsideCastlingAvailable = true;
    ret.blackQueensideCastlingAvailable = true;
    ret.enpassantTarget = Square();
    ret.halfMoveClock = 0;
    ret.fullMoveCount = 1;
    return ret;
}

QString BoardState::toFenString() const
{
    QString ret;
    for (int row=7;row>=0;--row) {
        for (int col=0;col<8;++col) {
            int emptyCount = 0;
            while (col < 8 && state[row][col] == ColouredPiece::None) {
                emptyCount++;
                col++;
            }
            if (emptyCount > 0) {
                ret += QString::number(emptyCount);
                col--;
            } else {
                ret += state[row][col].toFenString();
            }
        }
        if (row != 0)
            ret += "/";
    }
    ret += " ";
    ret += (activeColour == Colour::White) ? "w" : "b";
    ret += " ";
    if (!whiteKingsideCastlingAvailable &&
        !whiteQueensideCastlingAvailable &&
        !blackKingsideCastlingAvailable &&
!        blackQueensideCastlingAvailable) {
        ret += "-";
    } else {
        ret += (whiteKingsideCastlingAvailable) ? "K" : "";
        ret += (whiteQueensideCastlingAvailable) ? "Q" : "";
        ret += (blackKingsideCastlingAvailable) ? "k" : "";
        ret += (blackQueensideCastlingAvailable) ? "q" : "";
    }
    ret += " ";
    if (enpassantTarget == Square())
        ret += "-";
    else
        ret += enpassantTarget.toAlgebraicString();
    ret += " ";
    ret += QString::number(halfMoveClock);
    ret += " ";
    ret += QString::number(fullMoveCount);
    return ret;
}

QString BoardState::toString() const
{
    QString ret;
    ret += " abcdefgh ";
    ret += "\n";
    for (int row=7;row>=0;--row) {
        ret += QString::number(row + 1);
        for (int col=0;col<8;++col) {
            ret += state[row][col].toFenString();
        }
        ret += QString::number(row + 1);
        ret += "\n";
    }
    ret += " abcdefgh ";
    ret += "\n";
    ret += toFenString();
    return ret;
}

bool BoardState::isValid() const
{
    return fullMoveCount != -1;
}

bool BoardState::move(int fromRow, int fromCol, int toRow, int toCol, bool *promotion)
{
    if (promotion)
        *promotion = false;
    ColouredPiece piece = state[fromRow][fromCol];
    if (piece == ColouredPiece::None)
        return false;
    if (piece.colour() != activeColour)
        return false;
    bool capture = state[toRow][toCol] != ColouredPiece::None;
    state[fromRow][fromCol] = ColouredPiece::None;
    state[toRow][toCol] = piece;
    bool l_promotion = false;
    if (piece.piece() == Piece::Pawn &&
        toRow == enpassantTarget.row && toCol == enpassantTarget.col) {
        // en passant capture
        int captureRow = (toRow == 5) ? 4 : 3;
        Q_ASSERT(state[captureRow][toCol].piece() == Piece::Pawn);
        state[captureRow][toCol] = ColouredPiece::None;
    } else if (piece.piece() == Piece::Pawn &&
               ((toRow == 7 && piece.colour() == Colour::White) ||
                (toRow == 0 && piece.colour() == Colour::Black))) {
        l_promotion = true;
        if (promotion)
            *promotion = true;
    } else if (piece.piece() == Piece::King &&
               (fromRow == 0 || fromRow == 7) &&
               fromCol == 4 && (toCol == 2 || toCol == 6)) {
        // castling
        int rookFromCol, rookToCol;
        if (toCol == 2) {
            rookFromCol = 0;
            rookToCol = 3;
        } else if (toCol == 6) {
            rookFromCol = 7;
            rookToCol = 5;
        }
        ColouredPiece rook = state[fromRow][rookFromCol];
        Q_ASSERT(rook.piece() == Piece::Rook);
        state[fromRow][rookFromCol] = ColouredPiece::None;
        state[toRow][rookToCol] = rook;
    }
    if (piece.piece() == Piece::Pawn &&
        ((fromRow == 6 && toRow == 4) ||
         (fromRow == 1 && toRow == 3)) &&
        fromCol == toCol) {
        enpassantTarget = Square((toRow == 4) ? 5 : 2, toCol);
    } else {
        enpassantTarget = Square();
    }
    if (piece.piece() != Piece::Pawn && !capture)
        halfMoveClock++;
    else
        halfMoveClock = 0;
    if (piece == ColouredPiece::WhiteRook &&
        fromRow == 0 && fromCol == 7) {
        whiteKingsideCastlingAvailable = false;
    } else if (piece == ColouredPiece::WhiteRook &&
               fromRow == 0 && fromCol == 0) {
        whiteQueensideCastlingAvailable = false;
    } else if (piece == ColouredPiece::WhiteKing) {
        whiteKingsideCastlingAvailable = false;
        whiteQueensideCastlingAvailable = false;
    } else if (piece == ColouredPiece::BlackRook &&
        fromRow == 7 && fromCol == 7) {
        blackKingsideCastlingAvailable = false;
    } else if (piece == ColouredPiece::BlackRook &&
               fromRow == 7 && fromCol == 0) {
        blackQueensideCastlingAvailable = false;
    } else if (piece == ColouredPiece::BlackKing) {
        blackKingsideCastlingAvailable = false;
        blackQueensideCastlingAvailable = false;
    }
    if (!l_promotion) {
        activeColour = (activeColour == Colour::White) ? Colour::Black : Colour::White;
        if (activeColour == Colour::White)
            fullMoveCount++;
    }
    return true;
}

/**
 * @brief Is the given move legal in the current state?
 * @param fromRow 0-based row index of source square
 * @param fromCol 0-based column index of source square
 * @param toRow   0-based row index of target square
 * @param toCol   0-based column index of target square
 * @return @a true if the move is legal, else @a false
 * @note This method will return @a true for attempted king capture so long as the
 * move doesn't leave the current player in check.
 */
bool BoardState::isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    Q_ASSERT(fromRow >= 0 && fromRow <= 7 &&
             toRow >= 0 && toRow <= 7 &&
             fromCol >= 0 && fromCol <= 7 &&
             toCol >= 0 && toCol <= 7);
    const ColouredPiece& piece = state[fromRow][fromCol];
    if (piece == ColouredPiece::None || piece.colour() != activeColour)
        return false;
    if (!pieceCanMove(fromRow, fromCol, toRow, toCol))
        return false;
    if (state[toRow][toCol].piece() != Piece::King) {
        BoardState tmp = *this;
        tmp[toRow][toCol] = piece;
        tmp[fromRow][fromCol] = ColouredPiece::None;
        if (tmp.isCheck()) {
            // Illegal: leaves the king in check.
            return false;
        }
    }
    return true;
}

bool BoardState::isCheckmate() const
{
    // Check there is at least one move that doesn't leave the king in check.
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            const ColouredPiece& piece = state[row][col];
            if (piece != ColouredPiece::None && piece.colour() == activeColour) {
                for (int row2=0;row2<8;++row2) {
                    for (int col2=0;col2<8;++col2) {
                        if (isLegalMove(row, col, row2, col2))
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

bool BoardState::isCheck() const
{
    // Find the king of the current player.
    ColouredPiece king(activeColour, Piece::King);
    int kingRow = -1, kingCol = -1;
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            if (state[row][col] == king) {
                kingRow = row;
                kingCol = col;
                goto done;
            }
        }
    }
done:
    if (kingRow == -1)
        return false;
    Q_ASSERT(kingRow != -1 && kingCol != -1);
    // See if any opponent pieces could capture the king.
    BoardState tmp = *this;
    tmp.activeColour = (tmp.activeColour == Colour::White) ? Colour::Black : Colour::White;
    bool check = false;
    for (int row=0;row<8 && !check;++row) {
        for (int col=0;col<8 && !check;++col) {
            const ColouredPiece& piece = state[row][col];
            if (piece != ColouredPiece::None && piece.colour() != activeColour) {
                if (tmp.isLegalMove(row, col, kingRow, kingCol))
                    check = true;
            }
        }
    }
    return check;
}

bool BoardState::isAutomaticDraw(DrawReason *reason) const
{
    if (reason)
        *reason = DrawReason::None;
    // TODO
    return false;
}

bool BoardState::isClaimableDraw(DrawReason *reason) const
{
    if (reason)
        *reason = DrawReason::None;
    // TODO
    return false;
}

constexpr int sgn(int n)
{
    if (n < 0)
        return -1;
    else if (n > 0)
        return 1;
    else
        return 0;
}

bool BoardState::pieceCanMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    Q_ASSERT(fromRow >= 0 && fromRow <= 7 &&
             toRow >= 0 && toRow <= 7 &&
             fromCol >= 0 && fromCol <= 7 &&
             toCol >= 0 && toCol <= 7);
    const ColouredPiece& fromPiece = state[fromRow][fromCol];
    if (fromPiece == ColouredPiece::None)
        return false;
    const ColouredPiece& toPiece = state[toRow][toCol];
    if (toPiece != ColouredPiece::None && toPiece.colour() == fromPiece.colour())
        return false;
    if (fromRow == toRow && fromCol == toCol)
        return false;
    switch (fromPiece.piece()) {
    case Piece::Bishop:
        if (abs(toCol - fromCol) != abs(toRow - fromRow))
            return false;
        break;
    case Piece::King:
        if (fromCol == 4 && toCol == 6 && fromRow == 0 && toRow == 0) {
            if (!whiteKingsideCastlingAvailable)
                return false;
            if (state[0][7] != ColouredPiece(Colour::White, Piece::Rook))
                return false;
        } else if (fromCol == 4 && toCol == 2 && fromRow == 0 && toRow == 0) {
            if (!whiteQueensideCastlingAvailable)
                return false;
            if (state[0][0] != ColouredPiece(Colour::White, Piece::Rook))
                return false;
        } else if (fromCol == 4 && toCol == 6 && fromRow == 7 && toRow == 7) {
            if (!blackKingsideCastlingAvailable)
                return false;
            if (state[7][7] != ColouredPiece(Colour::Black, Piece::Rook))
                return false;
        } else if (fromCol == 4 && toCol == 2 && fromRow == 7 && toRow == 7) {
            if (!blackQueensideCastlingAvailable)
                return false;
            if (state[7][0] != ColouredPiece(Colour::Black, Piece::Rook))
                return false;
        } else if (abs(toCol - fromCol) > 1 || abs(toRow - fromRow) > 1) {
            return false;
        }
        break;
    case Piece::Knight:
        return (abs(toCol - fromCol) == 2 && abs(toRow - fromRow) == 1) ||
               (abs(toCol - fromCol) == 1 && abs(toRow - fromRow) == 2);
    case Piece::Pawn:
        if (fromPiece.colour() == Colour::White) {
            if (!(toPiece == ColouredPiece::None &&
                    fromCol == toCol &&
                    (toRow == fromRow + 1 || (fromRow == 1 && toRow == fromRow + 2)) ||
                   (((toPiece != ColouredPiece::None &&
                    toPiece.colour() != fromPiece.colour()) ||
                    (toRow == enpassantTarget.row && toCol == enpassantTarget.col)) &&
                    abs(toCol - fromCol) == 1 &&
                    toRow == fromRow + 1)))
                return false;
        } else {
            if (!(toPiece == ColouredPiece::None &&
                    fromCol == toCol &&
                    (toRow == fromRow - 1 || (fromRow == 6 && toRow == fromRow - 2)) ||
                   (((toPiece != ColouredPiece::None &&
                     toPiece.colour() != fromPiece.colour()) ||
                    (toRow == enpassantTarget.row && toCol == enpassantTarget.col)) &&
                    abs(toCol - fromCol) == 1 &&
                    toRow == fromRow - 1)))
                return false;
        }
        break;
    case Piece::Queen:
        if (abs(toCol - fromCol) != abs(toRow - fromRow) &&
               toCol != fromCol && toRow != fromRow)
            return false;
        break;
    case Piece::Rook:
        if (toCol != fromCol && toRow != fromRow)
            return false;
        break;
    }
    // Check the move doesn't go through another piece.
    assert(toCol - fromCol == 0 || toRow - fromRow == 0 || abs(toCol - fromCol) == abs(toRow - fromRow));
    if (fromPiece.piece() == Piece::King && abs(toCol - fromCol) > 1) {
        // castling
        // check empty king to rook
        int dir = sgn(toCol - fromCol);
        for (int col=fromCol+dir;col != 0 && col != 7;col+=dir) {
            if (state[fromRow][col] != ColouredPiece::None) {
                return false;
            }
        }
        // check king not in check in any intermediate squares
        for (int kingCol=fromCol+dir;kingCol!=toCol;kingCol+=dir) {
            for (int row=0;row<8;++row) {
                for (int col=0;col<8;++col) {
                    const ColouredPiece& piece = state[row][col];
                    if (piece != ColouredPiece::None && piece.colour() != activeColour) {
                        if (pieceCanMove(row, col, fromRow, kingCol))
                            return false;
                    }
                }
            }
        }
        return true;
    } else {
        int d = std::max(abs(toCol - fromCol), abs(toRow - fromRow));
        for (int i=1;i<d;++i) {
            int row = fromRow + sgn(toRow - fromRow) * i;
            int col = fromCol + sgn(toCol - fromCol) * i;
            if (state[row][col] != ColouredPiece::None)
                return false;
        }
    }
    return true;
}

bool BoardState::promote(Piece piece)
{
    if (piece == Piece::King || piece == Piece::Pawn)
        return false;
    int pawnRow = -1, pawnCol = -1;
    for (int col=0;col<8;++col) {
        if (state[0][col] != ColouredPiece::None &&
            state[0][col].piece() == Piece::Pawn) {
            pawnRow = 0;
            pawnCol = col;
        } else if (state[7][col] != ColouredPiece::None &&
                   state[7][col].piece() == Piece::Pawn) {
            pawnRow = 7;
            pawnCol = col;
        }
    }
    if (pawnRow == -1 || pawnCol == -1)
        return false;
    state[pawnRow][pawnCol] = ColouredPiece(state[pawnRow][pawnCol].colour(), piece);
    activeColour = (activeColour == Colour::White) ? Colour::Black : Colour::White;
    if (activeColour == Colour::White)
        fullMoveCount++;
    halfMoveClock = 0;
    return true;
}

BoardState BoardState::fromFenString(const QString& fen)
{
    BoardState ret;
    QStringList parts = fen.split(' ');
    if (parts.length() != 4 && parts.length() != 6)
        return BoardState();
    QString pieces = parts[0];
    int pos = 0;
    for (int row=7;row>=0;--row) {
        for (int col=0;col<8;++col) {
            QChar c = pieces[pos++];
            if (c >= '1' && c <= '9') {
                int emptyCount = c.toLatin1() - '0';
                if (col + emptyCount > 8)
                    return BoardState();
                for (int i=0;i<emptyCount;++i,col++) {
                    ret[row][col] = ColouredPiece::None;
                }
                col--;
            } else {
                auto piece = ColouredPiece::fromFenString(c);
                if (!piece.isValid())
                    return BoardState();
                ret[row][col] = piece;
            }
        }
        if (row != 0) {
            QChar c = pieces[pos++];
            Q_ASSERT(c == '/');
        }
    }
    ret.activeColour = (fen.section(' ', 1, 1) == "w") ? Colour::White : Colour::Black;
    QString castlingAvailable = parts[2];
    ret.whiteKingsideCastlingAvailable = castlingAvailable.contains("K");
    ret.whiteQueensideCastlingAvailable = castlingAvailable.contains("Q");
    ret.blackKingsideCastlingAvailable = castlingAvailable.contains("k");
    ret.blackQueensideCastlingAvailable = castlingAvailable.contains("q");
    ret.enpassantTarget = Square::fromAlgebraicString(parts[3]);
    if (parts.length() > 4) {
        bool ok = false;
        ret.halfMoveClock = parts[4].toInt(&ok);
        if (!ok)
            return BoardState();
        ret.fullMoveCount = parts[5].toInt(&ok);
        if (!ok)
            return BoardState();
    } else {
        ret.halfMoveClock = 0;
        ret.fullMoveCount = 1;
    }
    return ret;
}

bool BoardState::isLegal(IllegalBoardReason *reason) const
{
    bool legal = true;
    if (reason)
        *reason = IllegalBoardReason::None;
    int whiteKings = 0, blackKings = 0;
    int whitePawns = 0, blackPawns = 0;
    int whiteOtherPieces = 0, blackOtherPieces = 0;
    for (int row=0;row<8;++row) {
        for (int col=0;col<8;++col) {
            Chessboard::ColouredPiece piece = state[row][col];
            switch (piece) {
            case Chessboard::ColouredPiece::WhiteKing:
                whiteKings++;
                break;
            case Chessboard::ColouredPiece::BlackKing:
                blackKings++;
                break;
            case Chessboard::ColouredPiece::WhitePawn:
                whitePawns++;
                break;
            case Chessboard::ColouredPiece::BlackPawn:
                blackPawns++;
                break;
            default:
                if (piece.isValid() && piece.colour() == Colour::Black)
                    blackOtherPieces++;
                else if (piece.isValid() && piece.colour() == Colour::White)
                    whiteOtherPieces++;
                break;
            }
        }
    }
    if (whiteKings == 0) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::NoWhiteKing;
    } else if (whiteKings > 1) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::MoreThanOneWhiteKing;
    } else if (blackKings == 0) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::NoBlackKing;
    } else if (blackKings > 1) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::MoreThanOneBlackKing;
    } else if (whitePawns > 8) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::TooManyWhitePawns;
    } else if (blackPawns > 8) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::TooManyBlackPawns;
    } else if (whitePawns + whiteOtherPieces + whiteKings > 16) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::TooManyWhitePieces;
    } else if (blackPawns + blackOtherPieces + blackKings > 16) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::TooManyBlackPieces;
    }
    Chessboard::Colour otherColour =
        (activeColour == Chessboard::Colour::White) ?
            Chessboard::Colour::Black : Chessboard::Colour::White;
    Chessboard::BoardState copy = *this;
    copy.activeColour = otherColour;
    if (copy.isCheck()) {
        legal = false;
        if (reason)
            *reason = IllegalBoardReason::NonActivePlayerInCheck;
    }
    return legal;
}

}
