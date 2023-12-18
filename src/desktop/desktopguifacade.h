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

#ifndef DESKTOPGUIFACADE_H
#define DESKTOPGUIFACADE_H

#include "guifacade.h"
#include "syncdialog.h"

class ConnectingDialog;
class DiscoveryDialog;
class MainWindow;
class Options;

class DesktopGuiFacade : public GuiFacade
{
public:
    explicit DesktopGuiFacade(const Options& options, QObject *parent = nullptr);
    virtual ~DesktopGuiFacade();

public slots:
    void showConnectingPopup(const QString& address) override;
    void hideConnectingPopup() override;
    void showDiscoveryPopup() override;
    void addBoardToDiscoveryPopup(int index, const QString& address) override;
    void setConnectionState(ConnectionState state) override;
    void showNoBoardsDiscoveredPopup() override;
    void discoveryFinished() override;
    void showErrorPopup(const QString& errorMessage) override;
    void clearDiscovered() override;
    void setBoardState(const Chessboard::BoardState& newState) override;
    void showInvalidFenStringErrorPopup(const QString& fen) override;
    void showLocalNeedsSyncPopup() override;
    void showRemoteNeedsSyncPopup() override;
    void showPromotionPopup() override;
    void setGameProgress(const GameProgress& progress) override;
    void showGameOverPopup(const GameProgress& progress) override;
    void setActiveColour(Chessboard::Colour colour) override;
    void showDrawRequestedPopup(Chessboard::Colour requestor) override;
    void setEditMode(bool enabled) override;
    void showIllegalEditPopup(Chessboard::IllegalBoardReason reason) override;
    void showNewGameDialog() override;

private slots:
    void updateStatusMessage();

private:
    void showSyncPopup(SyncDialog::Direction direction);

    ConnectingDialog *m_connectingDialog;
    DiscoveryDialog *m_discoveryDialog;
    MainWindow *m_mainWindow;
    Chessboard::Colour m_activeColour { Chessboard::Colour::White };
    ConnectionState m_connectionState { ConnectionState::Connected };
    GameProgress m_progress { GameProgress::InProgress };
    const Options& m_options;
};

#endif // DESKTOPGUIFACADE_H
