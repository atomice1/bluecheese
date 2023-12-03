/*
 * bluecheese
 * Copyright (C) 2023 Chris January
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

namespace {
    enum LexState {
        Whitespace,
        String,
        StringEscape,
        SymbolOrInteger,
        Punctuation,
        InlineComment,
        LineComment,
        Escape,
        LexEof
    };
    enum ParseState {
        TagLeftBracket,
        TagSymbol,
        TagString,
        TagRightBracket,
        MoveNumber,
        MoveNumberDot,
        MoveWhite,
        MoveBlack,
        Termination,
        ParseEof
    };
}

Pgn PgnParser::parse(const QString& s, QString *errorMessage)
{
    Pgn ret;
    size_t i = 0;
    LexState lexState = Whitespace;
    ParseState parseState = TagLeftBracket;
    QByteArray currentToken;
    int lineno = 1, col = 0;
    int tokenLineno = 1, tokenCol = 1;
    QString symbol, value, moveText;
    AlgebraicNotation move;
    int moveNumber;
    bool ok, allDigits;
    while (i <= s.length() && parseState != ParseEof) {
        if (lexState == LexEof) {
            if (parseState == TagLeftBracket ||
                parseState == MoveNumber ||
                parseState == MoveBlack)
                break;
            if (errorMessage)
                *errorMessage = QString(QLatin1String("%1:%2: '%3': unexpected end-of-file")).arg(QString::number(tokenLineno), QString::number(tokenCol), qPrintable(currentToken));
            return Pgn();
        }
        col++;
        char c = i < s.length() ? s.at(i++).toLatin1() : '\0';
        LexState nextLexState = lexState;
        bool ignore = false;
        bool newToken = false;
        if (c == '\0') {
            ignore = true;
            newToken = true;
            nextLexState = LexEof;
        } else {
            switch (lexState) {
            case String:
                switch (c) {
                case '\\':
                    ignore = true;
                    break;
                case '"':
                    ignore = true;
                    newToken = true;
                    nextLexState = Whitespace;
                    break;
                default:
                    break;
                }
                break;
            case InlineComment:
                switch (c) {
                    case '}':
                        ignore = true;
                        newToken = true;
                        nextLexState = Whitespace;
                        break;
                    default:
                        break;
                }
                break;
            case LineComment:
            case Escape:
                switch (c) {
                case '\n':
                    ignore = true;
                    newToken = true;
                    nextLexState = Whitespace;
                    break;
                default:
                    break;
                }
                break;
            default:
                switch (c) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    newToken = lexState != Whitespace;
                    nextLexState = Whitespace;
                    break;
                case '"':
                    ignore = true;
                    newToken = true;
                    nextLexState = String;
                    break;
                case '{':
                    ignore = true;
                    newToken = true;
                    nextLexState = InlineComment;
                    break;
                case ';':
                case '%':
                    ignore = true;
                    newToken = true;
                    nextLexState = LineComment;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                default:
                    if (c >= '0' && c <= '9' ||
                        c >= 'A' && c <= 'Z' ||
                        c >= 'a' && c <= 'z' ||
                        c == '_' || c == '+' ||
                        c == '+' || c == '#' ||
                        c == '=' || c == ':' ||
                        c == '-' || c == '/') {
                        newToken = lexState != SymbolOrInteger;
                        nextLexState = SymbolOrInteger;
                    } else {
                        newToken = true;
                        nextLexState = Punctuation;
                    }
                }
            }
        }
        if (newToken &&
            lexState != Whitespace &&
            lexState != InlineComment &&
            lexState != LineComment &&
            lexState != Escape) {
            loop: switch (parseState) {
            case TagLeftBracket:
                if (lexState == SymbolOrInteger) {
                    parseState = MoveNumber;
                    goto loop;
                }
                if (currentToken != "[") {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': '[' expected")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                parseState = TagSymbol;
                break;
            case TagSymbol:
                if (lexState != SymbolOrInteger) {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': symbol (tag name) expected")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                symbol = QString::fromLatin1(currentToken);
                parseState = TagString;
                break;
            case TagString:
                if (lexState != String) {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': string (tag value) expected")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                value = QString::fromLatin1(currentToken);
                ret.tags[symbol] = value;
                parseState = TagRightBracket;
                break;
            case TagRightBracket:
                if (currentToken != "]") {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': ']' expected")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                parseState = TagLeftBracket;
                break;
            case MoveNumber:
                moveNumber = currentToken.toInt(&ok);
                if (lexState != SymbolOrInteger || !ok) {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': integer (move number) expected")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                parseState = MoveNumberDot;
                break;
            case MoveNumberDot:
                if (currentToken != ".") {
                    parseState = (ret.moves.size() & 1) ? MoveBlack : MoveWhite;
                    goto loop;
                }
                break;
            case MoveWhite:
            case MoveBlack:
                moveText = QString::fromLatin1(currentToken);
                if (moveText == "1-0" || moveText == "0-1" || moveText == "1/2-1/2" || moveText == "*") {
                    parseState = Termination;
                    goto loop;
                }
                allDigits = true;
                for (size_t i=0;i<moveText.length();++i) {
                    if (moveText.at(i) < '0' || moveText.at(i) >= '9') {
                        allDigits = false;
                        break;
                    }
                }
                if (allDigits) {
                    // This is actually a move number
                    parseState = MoveNumber;
                    goto loop;
                }
                move = AlgebraicNotation::fromString(moveText);
                if (!move.isValid()) {
                    if (errorMessage)
                        *errorMessage = QString(QLatin1String("%1:%2: '%3': invalid move")).arg(QString::number(tokenLineno), QString::number(tokenCol), currentToken);
                    return Pgn();
                }
                ret.moves.append(move);
                parseState = (parseState == MoveWhite) ? MoveBlack : MoveNumber;
                break;
            case Termination:
                parseState = ParseEof;
                break;
            case ParseEof:
                break;
            }
        }
        if (newToken) {
            lexState = nextLexState;
            currentToken.clear();
            tokenLineno = lineno;
            tokenCol = col;
        }
        if (!ignore)
            currentToken.append(c);
        if (c == '\n') {
            col = 0;
            lineno++;
        }
    }
    return ret;
}

Pgn PgnParser::parse(QIODevice *device, QString *errorMessage)
{
    QByteArray ba = device->readAll();
    QString s = QString::fromLatin1(ba);
    return parse(s, errorMessage);
}

}
