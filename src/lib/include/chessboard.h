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
#include <QIODevice>
#include <QMap>
#include <QObject>
#include <QQueue>
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

class LIBCHESSBOARD_EXPORT BoardAddress {
public:
    BoardAddress();
    BoardAddress(const BoardAddress& other);
    ~BoardAddress();
    ConnectionMethod connectionMethod() const;
    QString toString() const;
    QByteArray toByteArray() const;
    bool isValid() const;
    static BoardAddress fromByteArray(const QByteArray& array);
    BoardAddress& operator=(const BoardAddress& other);
    bool operator==(const BoardAddress& other) const;
    bool operator!=(const BoardAddress& other) const;
    static BoardAddress fromString(const QString& s);
private:
    BoardAddress(const BoardAddressPrivate *d);
    inline const BoardAddressPrivate *d_func() const { return d_ptr.get(); }
    QScopedPointer<const BoardAddressPrivate> d_ptr;
friend class BoardDiscoveryPrivate;
friend class ConnectionFactory;
};

class LIBCHESSBOARD_EXPORT BoardDiscovery : public QObject {
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
    BoardDiscovery(QObject *parent = nullptr);
    virtual ~BoardDiscovery();
    void start();
    void stop();
signals:
    void boardDiscovered(const Chessboard::BoardAddress& address);
    void boardsDiscovered(const QList<Chessboard::BoardAddress>& address);
    void error(BoardDiscovery::Error error);
    void finished();
    void noBoardsDiscovered();
    void started();
private:
    Q_DECLARE_PRIVATE(BoardDiscovery)
    QScopedPointer<BoardDiscoveryPrivate> d_ptr;
};

class LIBCHESSBOARD_EXPORT ConnectionManager : public QObject {
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

    ConnectionManager(QObject *parent = nullptr);
    virtual ~ConnectionManager();
public slots:
    void connectToBoard(const Chessboard::BoardAddress& address, QObject *parent = nullptr);
    void disconnectFromBoard();
signals:
    void connected(Chessboard::RemoteBoard *board);
    void error(Chessboard::ConnectionManager::Error error);
    void disconnected();
    void connectionFailed();
    void connecting(const Chessboard::BoardAddress& address);
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

LIBCHESSBOARD_EXPORT Piece pieceFromAlgebraicString(const QString& algebraicNotation);
LIBCHESSBOARD_EXPORT Piece pieceFromAlgebraicChar(QChar algebraicNotation);

enum class Colour {
    White = 0x10,
    Black = 0x20
};

inline Colour invertColour(Colour colour) {
    return (colour == Colour::White) ? Colour::Black : Colour::White;
}

class ColouredPiece {
public:
    static const ColouredPiece WhitePawn;
    static const ColouredPiece WhiteRook;
    static const ColouredPiece WhiteKnight;
    static const ColouredPiece WhiteBishop;
    static const ColouredPiece WhiteQueen;
    static const ColouredPiece WhiteKing;
    static const ColouredPiece BlackPawn;
    static const ColouredPiece BlackRook;
    static const ColouredPiece BlackKnight;
    static const ColouredPiece BlackBishop;
    static const ColouredPiece BlackQueen;
    static const ColouredPiece BlackKing;
    static const ColouredPiece None;

    constexpr ColouredPiece() : m_value(0) {}
    constexpr ColouredPiece(Colour colour, Piece piece) :
        m_value(static_cast<uint8_t>(colour) |static_cast<uint8_t>(piece)) {}
    constexpr ColouredPiece(const ColouredPiece& other) :
        m_value(other.m_value) {}

    constexpr Colour colour() const { return static_cast<Colour>(m_value & 0x30); }
    constexpr Piece piece() const { return static_cast<Piece>(m_value & 0xf); }
    constexpr bool isValid() const { return m_value != 0; };
    constexpr bool operator==(const ColouredPiece& other) { return m_value == other.m_value; }
    constexpr bool operator!=(const ColouredPiece& other) { return m_value != other.m_value; }
    constexpr operator int() const { return m_value; }
    ColouredPiece& operator=(const ColouredPiece& other) { m_value = other.m_value; return *this; }
    LIBCHESSBOARD_EXPORT QString toFenString() const;
    LIBCHESSBOARD_EXPORT QString toUnicode() const;
    LIBCHESSBOARD_EXPORT static ColouredPiece fromFenString(const QString& fen);
    LIBCHESSBOARD_EXPORT static ColouredPiece fromFenChar(QChar fen);
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

struct LIBCHESSBOARD_EXPORT Square {
    Square() : row(-1), col(-1) {}
    Square(int r, int c) : row(r), col(c) {}
    Square(const Square& other) : row(other.row), col(other.col) {}
    int row;
    int col;
    bool isValid() const { return row >= 0 && row < 8 & col >= 0 && col < 8; }
    bool operator==(const Square& other) const { return row == other.row && col == other.col; }
    bool operator!=(const Square& other) const { return row != other.row && col != other.col; }
    Square& operator=(const Square& other) { row = other.row; col = other.col; return *this; }
    bool operator<(const Square& other) const { return row < other.row || row == other.row && col < other.col; }
    bool operator>(const Square& other) const { return row > other.row || row == other.row && col > other.col; }
    QString toString() const;
    QString toAlgebraicString() const;
    static Square fromAlgebraicString(const QString& s);
};

enum class Castling {
    None,
    KingsideCastling,
    QueensideCastling
};

enum class CheckStatus {
    None,
    Check,
    Checkmate
};

class BoardState;
struct LIBCHESSBOARD_EXPORT AlgebraicNotation {
    AlgebraicNotation();
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    Piece piece;
    Castling castling;
    bool promotion;
    Piece promotionPiece;
    bool enPassant;
    bool capture;
    CheckStatus checkStatus;
    bool drawOffered;
    bool isValid() const { return (toRow >= 0 && toRow < 8) || (toCol >= 0 && toCol < 8) &&
                (fromRow == -1 || (fromRow >= 0 && fromRow < 8)) &&
                (fromCol == -1 || (fromCol >= 0 && fromCol < 8)); };
    AlgebraicNotation resolve(const BoardState& state) const;
    static AlgebraicNotation fromString(const QString& s);
};

enum class DrawReason {
    None,
    Stalemate,
    ThreefoldRepetitionRule,
    FiftyMoveRule,
    FivefoldRepetitionRule,
    SeventyFiveMoveRule,
    DeadPosition,
    MutualAgreement
};

enum class IllegalBoardReason {
    None,
    NoWhiteKing,
    NoBlackKing,
    MoreThanOneWhiteKing,
    MoreThanOneBlackKing,
    TooManyWhitePawns,
    TooManyBlackPawns,
    TooManyWhitePieces,
    TooManyBlackPieces,
    NonActivePlayerInCheck
};

struct LIBCHESSBOARD_EXPORT BoardState {
    BoardState();
    ColouredPiece state[8][8];
    Colour activeColour;
    bool whiteKingsideCastlingAvailable;
    bool whiteQueensideCastlingAvailable;
    bool blackKingsideCastlingAvailable;
    bool blackQueensideCastlingAvailable;
    Square enpassantTarget;
    int halfMoveClock;
    int fullMoveCount;
    QQueue<QByteArray> history;
    ColouredPiece *operator[](int row) {
        return reinterpret_cast<ColouredPiece *>(&state[row][0]);
    }
    const ColouredPiece *operator[](int row) const {
        return reinterpret_cast<const ColouredPiece *>(&state[row][0]);
    }
    ColouredPiece& operator[](const Square& square) {
        return state[square.row][square.col];
    }
    const ColouredPiece& operator[](const Square& square) const {
        return state[square.row][square.col];
    }
    QString toFenString() const;
    QString toString() const;
    bool isValid() const;
    bool move(int fromRow, int fromCol, int toRow, int toCol, bool *promotionRequired = nullptr);
    bool move(const Square& from, const Square& to, bool *promotionRequired = nullptr) {
        return move(from.row, from.col, to.row, to.col, promotionRequired);
    }
    bool move(const QString& algebraicNotation, bool *promotionRequired = nullptr);
    bool move(const AlgebraicNotation& algebraicNotation, bool *promotionRequired = nullptr);
    bool promote(Piece piece);
    bool isLegalMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isLegalMove(const Square& from, const Square& to) const {
        return isLegalMove(from.row, from.col, to.row, to.col);
    }
    bool hasLegalMove() const;
    bool isCheckmate() const;
    bool isCheck() const;
    bool isAutomaticDraw(DrawReason *reason = nullptr) const;
    bool isClaimableDraw(DrawReason *reason = nullptr) const;
    bool isPromotionRequired() const;
    bool isLegal(IllegalBoardReason *reason) const;
    QByteArray key() const;
    QList<QPair<Square, Square> > legalMoves() const;
    static BoardState fromFenString(const QString& fen);
    static BoardState newGame();
private:
    bool pieceCanMove(int fromRow, int fromCol, int toRow, int toCol) const;
};

enum class PlayerType {
    Human,
    Ai
};

enum class PlayerLocation {
    LocalBoard,
    LocalApp,
    Online
};

struct PlayerOptions {
    PlayerType playerType {PlayerType::Human};
    PlayerLocation playerLocation {PlayerLocation::LocalApp};
    int aiNominalElo {600};
};

struct GameOptions {
    PlayerOptions white;
    PlayerOptions black;
};

class LIBCHESSBOARD_EXPORT RemoteBoard : public QObject {
    Q_OBJECT
public:
    RemoteBoard(const BoardAddress& address, QObject *parent = nullptr);
    virtual ~RemoteBoard();
    BoardAddress address() const;
    virtual void requestRemoteBoardState();
    virtual void requestNewGame();
    virtual void requestNewGame(const GameOptions& gameOptions);
    virtual void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestMove(const Square& from, const Square& to) {
        requestMove(from.row, from.col, to.row, to.col);
    }
    virtual void requestPromotion(Piece piece);
    virtual void requestDraw(Colour requestor);
    virtual void requestResignation(Colour requestor);
    virtual void setBoardState(const BoardState& boardState);
signals:
    void remoteMove(int fromRow, int fromCol, int toRow, int toCol);
    void remotePromotion(Chessboard::Piece piece);
    void remoteBoardState(const Chessboard::BoardState& boardState);
    void remoteDrawRequested(Chessboard::Colour requestor);
    void remoteDraw(Chessboard::DrawReason reason);
    void remoteResignation(Chessboard::Colour colour);
    void remoteCheckmate(Chessboard::Colour winner);
protected:
    Q_DECLARE_PRIVATE(RemoteBoard);
    QScopedPointer<RemoteBoardPrivate> d_ptr;
};

struct LIBCHESSBOARD_EXPORT Pgn {
    QMap<QString, QString> tags;
    QList<AlgebraicNotation> moves;
    bool isValid() const { return !moves.isEmpty(); }
};

class LIBCHESSBOARD_EXPORT PgnParser {
public:
    Pgn parse(const QString& s, QString *errorMessage = nullptr);
    Pgn parse(QIODevice *file, QString *errorMessage = nullptr);
};

}

#endif // CHESSBOARD_H
