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

#ifndef GUIFACADE_H
#define GUIFACADE_H

#include <QObject>
#include "chessboard.h"
#include "connectionstate.h"
#include "gameprogress.h"

class GuiFacade : public QObject
{
    Q_OBJECT
public:
    GuiFacade(QObject *parent = nullptr);
    virtual ~GuiFacade();

signals:
    void connectRequested();
    void disconnectRequested();
    void cancelConnect();
    void restartDiscovery();
    void cancelDiscovery();
    void connectToDiscoveredBoard(int index);
    void sendFenRequested(const QString& fen);
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void syncFromLocalToRemote();
    void syncFromRemoteToLocal();
    void requestNewGame();
    void requestNewGameOptions(Chessboard::GameOptions gameOptions);
    void requestPromotion(Chessboard::Piece piece);
    void requestDraw(Chessboard::Colour requestor);
    void requestResignation(Chessboard::Colour requestor);
    void requestEdit(const Chessboard::BoardState& state);

public slots:
    virtual void showConnectingPopup(const QString& address) = 0;
    virtual void hideConnectingPopup() = 0;
    virtual void showDiscoveryPopup() = 0;
    virtual void addBoardToDiscoveryPopup(int index, const QString& address) = 0;
    virtual void setConnectionState(ConnectionState state) = 0;
    virtual void showNoBoardsDiscoveredPopup() = 0;
    virtual void discoveryFinished() = 0;
    virtual void showErrorPopup(const QString& errorMessage) = 0;
    virtual void clearDiscovered() = 0;
    virtual void setBoardState(const Chessboard::BoardState& newState) = 0;
    virtual void showInvalidFenStringErrorPopup(const QString& fen) = 0;
    virtual void showLocalNeedsSyncPopup() = 0;
    virtual void showRemoteNeedsSyncPopup() = 0;
    virtual void showPromotionPopup() = 0;
    virtual void setGameProgress(const GameProgress& progress) = 0;
    virtual void showGameOverPopup(const GameProgress& progress) = 0;
    virtual void setActiveColour(Chessboard::Colour colour) = 0;
    virtual void showDrawRequestedPopup(Chessboard::Colour requestor) = 0;
    virtual void setEditMode(bool enabled) = 0;
    virtual void showIllegalEditPopup(Chessboard::IllegalBoardReason reason) = 0;
    virtual void showNewGameDialog() = 0;
    virtual void gameOptionsChanged(const Chessboard::GameOptions& gameOptions) = 0;
    virtual void showBluetoothPermissionDeniedPopup() = 0;
};

#endif // GUIFACADE_H
