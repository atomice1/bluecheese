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

#ifndef APPLICATIONFACADE_H
#define APPLICATIONFACADE_H

#include <QObject>
#include <QSettings>

#include "chessboard.h"
#include "gameprogress.h"

class AiController;
class CompositeBoard;

class ApplicationFacade : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationFacade(QObject *parent = nullptr);
    QSettings *settings() { return &m_settings; }

signals:
    void connected(Chessboard::RemoteBoard *board);
    void disconnected();
    void error(const QString& errorMessage);
    void remoteMove(int fromRow, int fromCol, int toRow, int toCol);
    void remoteBoardState(const Chessboard::BoardState& newState);
    void discoveryStarted();
    void discoveryFinished();
    void boardDiscovered(const Chessboard::BoardAddress& address);
    void boardsDiscovered(const QList<Chessboard::BoardAddress>& addresses);
    void connecting(const Chessboard::BoardAddress& address);
    void noBoardsDiscovered();
    void connectionFailed();
    void noLastConnectedAddress();
    void invalidFenString(const QString& fen);
    void illegalMove(int fromRow, int fromCol, int toRow, int toCol);
    void localOutOfSyncWithRemote();
    void remoteOutOfSyncWithLocal();
    void boardStateChanged(const Chessboard::BoardState& newState);
    void promotionRequired();
    void drawRequested(Chessboard::Colour requestor);
    void draw(Chessboard::DrawReason reason);
    void resignation(Chessboard::Colour colour);
    void checkmate(Chessboard::Colour winner);
    void gameProgressChanged(const GameProgress& progress);
    void activeColourChanged(Chessboard::Colour colour);
    void gameOver();

public slots:
    virtual void connectToLast();
    virtual void connectToLastOrDiscover();
    virtual void connectToBoard(const Chessboard::BoardAddress& address);
    virtual void startDiscovery();
    virtual void disconnectFromBoard();
    virtual void stopDiscovery();
    virtual void setBoardStateFromFen(const QString& fen);
    virtual void requestRemoteBoardState();
    virtual void sendLocalBoardState();
    virtual void requestNewGame();
    virtual void requestNewGameOptions(const Chessboard::GameOptions& gameOptions);
    virtual void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    virtual void requestDraw(Chessboard::Colour requestor);
    virtual void requestResignation(Chessboard::Colour requestor);
    virtual void requestPromotion(Chessboard::Piece piece);

protected slots:
    virtual void onConnectionError(Chessboard::ConnectionManager::Error error);
    virtual void onDiscoveryError(Chessboard::BoardDiscovery::Error error);

private slots:
    virtual void setLastConnectedAddress(const Chessboard::BoardAddress& address);

private:
    QSettings m_settings;
    Chessboard::BoardAddress m_lastConnectedAddress;
    Chessboard::ConnectionManager *m_connectionManager {};
    Chessboard::BoardDiscovery *m_boardDiscovery {};
    CompositeBoard *m_board {};
    AiController *m_aiController {};
    Chessboard::GameOptions m_gameOptions;

    bool isCurrentPlayerAppAi() const;
    bool isPlayerAppAi(Chessboard::Colour colour) const;
friend class MockApplicationFacade;
};

#endif // APPLICATIONBASE_H
