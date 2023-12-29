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

#ifndef GUIAPPLICATIONBASE_H
#define GUIAPPLICATIONBASE_H

#include "applicationbase.h"
#include "chessboard.h"

class GameProgress;
class GuiFacade;

class GuiApplicationBase : public ApplicationBase
{
public:
    GuiApplicationBase(GuiFacade *guiFacade, const Options &options, QObject *parent = nullptr);
    virtual ~GuiApplicationBase();

    GuiFacade *guiFacade() { return m_guiFacade; }

protected slots:
    virtual void onConnected(Chessboard::RemoteBoard *board);
    virtual void onDisconnected();
    virtual void onError(const QString& errorMessage);
    virtual void onDiscoveryFinished();
    virtual void onDiscoveryStarted();
    virtual void onBoardDiscovered(const Chessboard::BoardAddress& address);
    virtual void onConnecting(const Chessboard::BoardAddress& address);
    virtual void onNoBoardsDiscovered();
    virtual void onConnectionFailed();
    virtual void onBoardStateChanged(const Chessboard::BoardState& newState);
    virtual void onLocalOutOfSyncWithRemote();
    virtual void onRemoteOutOfSyncWithLocal();
    virtual void onDrawRequested(Chessboard::Colour requestor);
    virtual void onActiveColourChanged(Chessboard::Colour colour);
    virtual void onGameProgressChanged(const GameProgress& progress);
    virtual void onPromotionRequired();

    virtual void onConnectRequested();
    virtual void onDisconnectRequested();
    virtual void onCancelConnect();
    virtual void onRestartDiscovery();
    virtual void onCancelDiscovery();
    virtual void onConnectToDiscoveredBoard(int index);
    virtual void onSendFenRequested(const QString& fen);
    virtual void onInvalidFenString(const QString& fen);
    virtual void onSyncFromLocalToRemote();
    virtual void onSyncFromRemoteToLocal();
    virtual void onRequestMove(int fromRow, int fromCol, int toRow, int toCol);
    virtual void onRequestNewGame(const Chessboard::GameOptions& gameOptions);
    virtual void onRequestDraw(Chessboard::Colour requestor);
    virtual void onRequestResignation(Chessboard::Colour requestor);
    virtual void onRequestPromotion(Chessboard::Piece piece);
    virtual void onRequestEdit(const Chessboard::BoardState& state);
    virtual void autoConnect();

private:
    void maybeShowNewGameDialog();

private:
    GuiFacade *m_guiFacade {};
    QList<Chessboard::BoardAddress> m_discovered;
    bool m_pendingNewGame { true };
    bool m_connected {};
};

#endif // GUIAPPLICATIONBASE_H
