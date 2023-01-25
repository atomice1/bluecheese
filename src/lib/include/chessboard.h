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

#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QByteArray>
#include <QObject>
#include <QSharedDataPointer>
#include <QString>

#include <utility>

#include "chessboard_global.h"

namespace Chessboard {

class BoardAddressPrivate;
class BoardDiscoveryPrivate;
class Board;
class BoardPrivate;
class ConnectionManager;
class ConnectionManagerPrivate;
class RemoteBoard;
class RemoteBoardPrivate;

enum ConnectionMethod {
    CONNECTION_BLE = 1
};

class BoardAddress {
public:
    LIBCHESSBOARD_EXPORT BoardAddress();
    LIBCHESSBOARD_EXPORT BoardAddress(const BoardAddress& other);
    LIBCHESSBOARD_EXPORT ~BoardAddress();
    LIBCHESSBOARD_EXPORT ConnectionMethod connectionMethod() const;
    LIBCHESSBOARD_EXPORT QString toString() const;
    LIBCHESSBOARD_EXPORT QByteArray toByteArray() const;
    LIBCHESSBOARD_EXPORT bool isValid() const;
    LIBCHESSBOARD_EXPORT static BoardAddress fromByteArray(const QByteArray& array);
    LIBCHESSBOARD_EXPORT BoardAddress& operator=(const BoardAddress& other);
    LIBCHESSBOARD_EXPORT bool operator==(const BoardAddress& other) const;
    LIBCHESSBOARD_EXPORT bool operator!=(const BoardAddress& other) const;
    LIBCHESSBOARD_EXPORT static BoardAddress fromString(const QString& s);
private:
    BoardAddress(const BoardAddressPrivate *d);
    inline const BoardAddressPrivate *d_func() const { return d_ptr.get(); }
    QScopedPointer<const BoardAddressPrivate> d_ptr;
friend class BoardDiscoveryPrivate;
friend class ConnectionFactory;
};

class BoardDiscovery : public QObject {
    Q_OBJECT
public:
    enum Error {
        NoError,
        PoweredOffError,
        InputOutputError,
        InvalidBluetoothAdapterError,
        UnsupportedPlatformError,
        UnsupportedDiscoveryMethod,
        LocationServiceTurnedOffError,
        MissingPermissionsError,
        UnknownError = 100
    };
    LIBCHESSBOARD_EXPORT BoardDiscovery(QObject *parent = nullptr);
    LIBCHESSBOARD_EXPORT virtual ~BoardDiscovery();
    LIBCHESSBOARD_EXPORT void start();
    LIBCHESSBOARD_EXPORT void stop();
signals:
    LIBCHESSBOARD_EXPORT void boardDiscovered(const BoardAddress& address);
    LIBCHESSBOARD_EXPORT void boardsDiscovered(const QList<BoardAddress>& address);
    LIBCHESSBOARD_EXPORT void error(Error error);
    LIBCHESSBOARD_EXPORT void finished();
    LIBCHESSBOARD_EXPORT void noBoardsDiscovered();
    LIBCHESSBOARD_EXPORT void started();
private:
    Q_DECLARE_PRIVATE(BoardDiscovery)
    QScopedPointer<BoardDiscoveryPrivate> d_ptr;
};

class ConnectionManager : public QObject {
    Q_OBJECT
public:
    enum Error {
        NoError,
        UnknownRemoteDeviceError,
        NetworkError,
        InvalidBluetoothAdapterError,
        ConnectionError,
        AdvertisingError,
        RemoteHostClosedError,
        AuthorizationError,
        MissingPermissionsError,
        OperationError,
        CharacteristicWriteError,
        DescriptorWriteError,
        CharacteristicReadError,
        DescriptorReadError,
        UnknownError = 100,
        NotSupported = 200,
        DisconnectDuringServiceDiscovery,
        DisconnectDuringCharacteristicDiscovery
    };

    LIBCHESSBOARD_EXPORT ConnectionManager(QObject *parent = nullptr);
    LIBCHESSBOARD_EXPORT virtual ~ConnectionManager();
public slots:
    LIBCHESSBOARD_EXPORT void connectToBoard(const BoardAddress& address, QObject *parent = nullptr);
    LIBCHESSBOARD_EXPORT void disconnectFromBoard();
signals:
    LIBCHESSBOARD_EXPORT void connected(RemoteBoard *board);
    LIBCHESSBOARD_EXPORT void error(Error error);
    LIBCHESSBOARD_EXPORT void disconnected();
    LIBCHESSBOARD_EXPORT void connectionFailed();
    LIBCHESSBOARD_EXPORT void connecting(const BoardAddress& address);
private:
    Q_DECLARE_PRIVATE(ConnectionManager);
    QScopedPointer<ConnectionManagerPrivate> d_ptr;
friend class BoardFactory;
friend class BoardDiscovery;
};

enum class Piece {
    Pawn   = 1,
    Rook   = 2,
    Knight = 3,
    Bishop = 4,
    Queen  = 5,
    King   = 6
};

enum class Colour {
    White = 0x10,
    Black = 0x20
};

inline Colour invertColour(Colour colour) {
    return (colour == Colour::White) ? Colour::Black : Colour::White;
}

class ColouredPiece {
public:
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhitePawn;
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhiteRook;
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhiteKnight;
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhiteBishop;
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhiteQueen;
    LIBCHESSBOARD_EXPORT static const ColouredPiece WhiteKing;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackPawn;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackRook;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackKnight;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackBishop;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackQueen;
    LIBCHESSBOARD_EXPORT static const ColouredPiece BlackKing;
    LIBCHESSBOARD_EXPORT static const ColouredPiece None;

    LIBCHESSBOARD_EXPORT constexpr ColouredPiece() : m_value(0) {}
    LIBCHESSBOARD_EXPORT constexpr ColouredPiece(Colour colour, Piece piece) :
        m_value(static_cast<uint8_t>(colour) |static_cast<uint8_t>(piece)) {}
    LIBCHESSBOARD_EXPORT constexpr ColouredPiece(const ColouredPiece& other) :
        m_value(other.m_value) {}

    LIBCHESSBOARD_EXPORT constexpr Colour colour() const { return static_cast<Colour>(m_value & 0x30); }
    LIBCHESSBOARD_EXPORT constexpr Piece piece() const { return static_cast<Piece>(m_value & 0xf); }
    LIBCHESSBOARD_EXPORT constexpr bool isValid() const { return m_value != 0; };
    LIBCHESSBOARD_EXPORT constexpr bool operator==(const ColouredPiece& other) { return m_value == other.m_value; }
    LIBCHESSBOARD_EXPORT constexpr bool operator!=(const ColouredPiece& other) { return m_value != other.m_value; }
    LIBCHESSBOARD_EXPORT constexpr operator int() const { return m_value; }
    LIBCHESSBOARD_EXPORT ColouredPiece& operator=(const ColouredPiece& other) { m_value = other.m_value; return *this; }
    LIBCHESSBOARD_EXPORT QString toFenString() const;
    LIBCHESSBOARD_EXPORT QString toUnicode() const;
    static LIBCHESSBOARD_EXPORT ColouredPiece fromFenString(const QString& fen);
    static LIBCHESSBOARD_EXPORT ColouredPiece fromFenChar(QChar fen);
private:
    uint8_t m_value;
};

inline constexpr ColouredPiece ColouredPiece::WhitePawn = ColouredPiece(Colour::White, Piece::Pawn);
inline constexpr ColouredPiece ColouredPiece::WhiteRook = ColouredPiece(Colour::White, Piece::Rook);
inline constexpr ColouredPiece ColouredPiece::WhiteKnight = ColouredPiece(Colour::White, Piece::Knight);
inline constexpr ColouredPiece ColouredPiece::WhiteBishop = ColouredPiece(Colour::White, Piece::Bishop);
inline constexpr ColouredPiece ColouredPiece::WhiteQueen = ColouredPiece(Colour::White, Piece::Queen);
inline constexpr ColouredPiece ColouredPiece::WhiteKing = ColouredPiece(Colour::White, Piece::King);
inline constexpr ColouredPiece ColouredPiece::BlackPawn = ColouredPiece(Colour::Black, Piece::Pawn);
inline constexpr ColouredPiece ColouredPiece::BlackRook = ColouredPiece(Colour::Black, Piece::Rook);
inline constexpr ColouredPiece ColouredPiece::BlackKnight = ColouredPiece(Colour::Black, Piece::Knight);
inline constexpr ColouredPiece ColouredPiece::BlackBishop = ColouredPiece(Colour::Black, Piece::Bishop);
inline constexpr ColouredPiece ColouredPiece::BlackQueen = ColouredPiece(Colour::Black, Piece::Queen);
inline constexpr ColouredPiece ColouredPiece::BlackKing = ColouredPiece(Colour::Black, Piece::King);
inline constexpr ColouredPiece ColouredPiece::None = ColouredPiece();

struct Square {
    LIBCHESSBOARD_EXPORT Square() : row(-1), col(-1) {}
    LIBCHESSBOARD_EXPORT Square(int r, int c) : row(r), col(c) {}
    LIBCHESSBOARD_EXPORT Square(const Square& other) : row(other.row), col(other.col) {}
    int row;
    int col;
    LIBCHESSBOARD_EXPORT bool isValid() const { return row >= 0 && row < 8 & col >= 0 && col < 8; }
    LIBCHESSBOARD_EXPORT bool operator==(const Square& other) const { return row == other.row && col == other.col; }
    LIBCHESSBOARD_EXPORT bool operator!=(const Square& other) const { return row != other.row && col != other.col; }
    LIBCHESSBOARD_EXPORT Square& operator=(const Square& other) { row = other.row; col = other.col; return *this; }
    LIBCHESSBOARD_EXPORT bool operator<(const Square& other) const { return row < other.row || row == other.row && col < other.col; }
    LIBCHESSBOARD_EXPORT bool operator>(const Square& other) const { return row > other.row || row == other.row && col > other.col; }
    LIBCHESSBOARD_EXPORT QString toString() const;
    LIBCHESSBOARD_EXPORT QString toAlgebraicString() const;
    static LIBCHESSBOARD_EXPORT Square fromAlgebraicString(const QString& s);
};

enum class DrawReason {
    None,
    Stalemate,
    ThreefoldRepetitionRule,
    FiftyMoveRule,
    FivefoldRepetition,
    SeventyFiveMoveRule,
    DeadPosition,
    MutualAgreement
};

struct BoardState {
    LIBCHESSBOARD_EXPORT BoardState();
    ColouredPiece state[8][8];
    Colour activeColour;
    bool whiteKingsideCastlingAvailable;
    bool whiteQueensideCastlingAvailable;
    bool blackKingsideCastlingAvailable;
    bool blackQueensideCastlingAvailable;
    Square enpassantTarget;
    int halfMoveClock;
    int fullMoveCount;
    LIBCHESSBOARD_EXPORT ColouredPiece *operator[](int row) {
        return reinterpret_cast<ColouredPiece *>(&state[row][0]);
    }
    LIBCHESSBOARD_EXPORT const ColouredPiece *operator[](int row) const {
        return reinterpret_cast<const ColouredPiece *>(&state[row][0]);
    }
    LIBCHESSBOARD_EXPORT ColouredPiece& operator[](const Square& square) {
        return state[square.row][square.col];
    }
    LIBCHESSBOARD_EXPORT const ColouredPiece& operator[](const Square& square) const {
        return state[square.row][square.col];
    }
    LIBCHESSBOARD_EXPORT QString toFenString() const;
    LIBCHESSBOARD_EXPORT QString toString() const;
    LIBCHESSBOARD_EXPORT bool isValid() const;
    LIBCHESSBOARD_EXPORT bool move(int fromRow, int fromCol, int toRow, int toCol, bool *promotionRequired = nullptr);
    LIBCHESSBOARD_EXPORT bool move(const Square& from, const Square& to, bool *promotionRequired = nullptr) {
        return move(from.row, from.col, to.row, to.col, promotionRequired);
    }
    LIBCHESSBOARD_EXPORT bool promote(Piece piece);
    LIBCHESSBOARD_EXPORT bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const;
    LIBCHESSBOARD_EXPORT bool isLegalMove(const Square& from, const Square& to) const {
        return isLegalMove(from.row, from.col, to.row, to.col);
    }
    LIBCHESSBOARD_EXPORT bool isCheckmate() const;
    LIBCHESSBOARD_EXPORT bool isCheck() const;
    LIBCHESSBOARD_EXPORT bool isAutomaticDraw(DrawReason *reason = nullptr) const;
    LIBCHESSBOARD_EXPORT bool isClaimableDraw(DrawReason *reason = nullptr) const;
    LIBCHESSBOARD_EXPORT bool isPromotionRequired() const;
    static LIBCHESSBOARD_EXPORT BoardState fromFenString(const QString& fen);
    static LIBCHESSBOARD_EXPORT BoardState newGame();
private:
    bool pieceCanMove(int fromRow, int fromCol, int toRow, int toCol) const;
};

class RemoteBoard : public QObject {
    Q_OBJECT
public:
    RemoteBoard(const BoardAddress& address, QObject *parent = nullptr);
    virtual ~RemoteBoard();
    LIBCHESSBOARD_EXPORT BoardAddress address() const;
    LIBCHESSBOARD_EXPORT virtual void requestRemoteBoardState();
    LIBCHESSBOARD_EXPORT virtual void requestNewGame();
    LIBCHESSBOARD_EXPORT virtual void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    LIBCHESSBOARD_EXPORT void requestMove(const Square& from, const Square& to) {
        requestMove(from.row, from.col, to.row, to.col);
    }
    LIBCHESSBOARD_EXPORT virtual void requestPromotion(Piece piece);
    LIBCHESSBOARD_EXPORT virtual void requestDraw(Colour requestor);
    LIBCHESSBOARD_EXPORT virtual void requestResignation(Colour requestor);
    LIBCHESSBOARD_EXPORT virtual void setBoardState(const BoardState& boardState);
signals:
    LIBCHESSBOARD_EXPORT void remoteMove(int fromRow, int fromCol, int toRow, int toCol);
    LIBCHESSBOARD_EXPORT void remotePromotion(Piece piece);
    LIBCHESSBOARD_EXPORT void remoteBoardState(const BoardState& boardState);
    LIBCHESSBOARD_EXPORT void remoteDrawRequested(Colour requestor);
    LIBCHESSBOARD_EXPORT void remoteDraw(DrawReason reason);
    LIBCHESSBOARD_EXPORT void remoteResignation(Colour colour);
    LIBCHESSBOARD_EXPORT void remoteCheckmate(Colour winner);
protected:
    Q_DECLARE_PRIVATE(RemoteBoard);
    QScopedPointer<RemoteBoardPrivate> d_ptr;
};

}

#endif // CHESSBOARD_H
