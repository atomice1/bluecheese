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

#include <QCoreApplication>
#include "commontranslations.h"

QString colourToString(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return QCoreApplication::translate("CommonTranslations", "white");
    case Chessboard::Colour::Black:
        return QCoreApplication::translate("CommonTranslations", "black");
    default:
        Q_ASSERT(colour == Chessboard::Colour::White ||
                 colour == Chessboard::Colour::Black);
        return QString();
    }
}

QString colourToStringTitle(Chessboard::Colour colour)
{
    switch (colour) {
    case Chessboard::Colour::White:
        return QCoreApplication::translate("CommonTranslations", "White");
    case Chessboard::Colour::Black:
        return QCoreApplication::translate("CommonTranslations", "Black");
    default:
        Q_ASSERT(colour == Chessboard::Colour::White ||
                 colour == Chessboard::Colour::Black);
        return QString();
    }
}

QString pieceToString(Chessboard::Piece piece)
{
    switch (piece) {
    case Chessboard::Piece::Bishop:
        return QCoreApplication::translate("CommonTranslations", "bishop");
    case Chessboard::Piece::Pawn:
        return QCoreApplication::translate("CommonTranslations", "pawn");
    case Chessboard::Piece::Rook:
        return QCoreApplication::translate("CommonTranslations", "rook");
    case Chessboard::Piece::Knight:
        return QCoreApplication::translate("CommonTranslations", "knight");
    case Chessboard::Piece::Queen:
        return QCoreApplication::translate("CommonTranslations", "queen");
    case Chessboard::Piece::King:
        return QCoreApplication::translate("CommonTranslations", "king");
    default:
        Q_ASSERT(piece == Chessboard::Piece::Bishop ||
                 piece == Chessboard::Piece::Pawn ||
                 piece == Chessboard::Piece::Rook ||
                 piece == Chessboard::Piece::Knight ||
                 piece == Chessboard::Piece::Queen ||
                 piece == Chessboard::Piece::King);
        return QString();
    }
}

QString pieceToStringTitle(Chessboard::Piece piece)
{
    switch (piece) {
    case Chessboard::Piece::Bishop:
        return QCoreApplication::translate("CommonTranslations", "Bishop");
    case Chessboard::Piece::Pawn:
        return QCoreApplication::translate("CommonTranslations", "Pawn");
    case Chessboard::Piece::Rook:
        return QCoreApplication::translate("CommonTranslations", "Rook");
    case Chessboard::Piece::Knight:
        return QCoreApplication::translate("CommonTranslations", "Knight");
    case Chessboard::Piece::Queen:
        return QCoreApplication::translate("CommonTranslations", "Queen");
    case Chessboard::Piece::King:
        return QCoreApplication::translate("CommonTranslations", "King");
    default:
        Q_ASSERT(piece == Chessboard::Piece::Bishop ||
                 piece == Chessboard::Piece::Pawn ||
                 piece == Chessboard::Piece::Rook ||
                 piece == Chessboard::Piece::Knight ||
                 piece == Chessboard::Piece::Queen ||
                 piece == Chessboard::Piece::King);
        return QString();
    }
}

QString colouredPieceToString(Chessboard::ColouredPiece piece)
{
    switch (piece.colour()) {
    case Chessboard::Colour::White:
        switch (piece.piece()) {
        case Chessboard::Piece::Bishop:
            return QCoreApplication::translate("CommonTranslations", "white bishop");
        case Chessboard::Piece::Pawn:
            return QCoreApplication::translate("CommonTranslations", "white pawn");
        case Chessboard::Piece::Rook:
            return QCoreApplication::translate("CommonTranslations", "white rook");
        case Chessboard::Piece::Knight:
            return QCoreApplication::translate("CommonTranslations", "white knight");
        case Chessboard::Piece::Queen:
            return QCoreApplication::translate("CommonTranslations", "white queen");
        case Chessboard::Piece::King:
            return QCoreApplication::translate("CommonTranslations", "white king");
        default:
            Q_ASSERT(piece.piece() == Chessboard::Piece::Bishop ||
                     piece.piece() == Chessboard::Piece::Pawn ||
                     piece.piece() == Chessboard::Piece::Rook ||
                     piece.piece() == Chessboard::Piece::Knight ||
                     piece.piece() == Chessboard::Piece::Queen ||
                     piece.piece() == Chessboard::Piece::King);
            return QString();
        }
    case Chessboard::Colour::Black:
        switch (piece.piece()) {
        case Chessboard::Piece::Bishop:
            return QCoreApplication::translate("CommonTranslations", "black bishop");
        case Chessboard::Piece::Pawn:
            return QCoreApplication::translate("CommonTranslations", "black pawn");
        case Chessboard::Piece::Rook:
            return QCoreApplication::translate("CommonTranslations", "black rook");
        case Chessboard::Piece::Knight:
            return QCoreApplication::translate("CommonTranslations", "black knight");
        case Chessboard::Piece::Queen:
            return QCoreApplication::translate("CommonTranslations", "black queen");
        case Chessboard::Piece::King:
            return QCoreApplication::translate("CommonTranslations", "black king");
        default:
            Q_ASSERT(piece.piece() == Chessboard::Piece::Bishop ||
                     piece.piece() == Chessboard::Piece::Pawn ||
                     piece.piece() == Chessboard::Piece::Rook ||
                     piece.piece() == Chessboard::Piece::Knight ||
                     piece.piece() == Chessboard::Piece::Queen ||
                     piece.piece() == Chessboard::Piece::King);
            return QString();
        }
    default:
        Q_ASSERT(piece.colour() == Chessboard::Colour::White ||
                 piece.colour() == Chessboard::Colour::Black);
        return QString();
    }
}

QString colouredPieceToStringTitle(Chessboard::ColouredPiece piece)
{
    switch (piece.colour()) {
    case Chessboard::Colour::White:
        switch (piece.piece()) {
        case Chessboard::Piece::Bishop:
            return QCoreApplication::translate("CommonTranslations", "White Bishop");
        case Chessboard::Piece::Pawn:
            return QCoreApplication::translate("CommonTranslations", "White Pawn");
        case Chessboard::Piece::Rook:
            return QCoreApplication::translate("CommonTranslations", "White Rook");
        case Chessboard::Piece::Knight:
            return QCoreApplication::translate("CommonTranslations", "White Knight");
        case Chessboard::Piece::Queen:
            return QCoreApplication::translate("CommonTranslations", "White Queen");
        case Chessboard::Piece::King:
            return QCoreApplication::translate("CommonTranslations", "White King");
        default:
            Q_ASSERT(piece.piece() == Chessboard::Piece::Bishop ||
                     piece.piece() == Chessboard::Piece::Pawn ||
                     piece.piece() == Chessboard::Piece::Rook ||
                     piece.piece() == Chessboard::Piece::Knight ||
                     piece.piece() == Chessboard::Piece::Queen ||
                     piece.piece() == Chessboard::Piece::King);
            return QString();
        }
    case Chessboard::Colour::Black:
        switch (piece.piece()) {
        case Chessboard::Piece::Bishop:
            return QCoreApplication::translate("CommonTranslations", "Black Bishop");
        case Chessboard::Piece::Pawn:
            return QCoreApplication::translate("CommonTranslations", "Black Pawn");
        case Chessboard::Piece::Rook:
            return QCoreApplication::translate("CommonTranslations", "Black Rook");
        case Chessboard::Piece::Knight:
            return QCoreApplication::translate("CommonTranslations", "Black Knight");
        case Chessboard::Piece::Queen:
            return QCoreApplication::translate("CommonTranslations", "Black Queen");
        case Chessboard::Piece::King:
            return QCoreApplication::translate("CommonTranslations", "Black King");
        default:
            Q_ASSERT(piece.piece() == Chessboard::Piece::Bishop ||
                     piece.piece() == Chessboard::Piece::Pawn ||
                     piece.piece() == Chessboard::Piece::Rook ||
                     piece.piece() == Chessboard::Piece::Knight ||
                     piece.piece() == Chessboard::Piece::Queen ||
                     piece.piece() == Chessboard::Piece::King);
            return QString();
        }
    default:
        Q_ASSERT(piece.colour() == Chessboard::Colour::White ||
                 piece.colour() == Chessboard::Colour::Black);
        return QString();
    }
}
