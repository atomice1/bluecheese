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

#include <QGuiApplication>
#include <QMessageBox>
#include "commontranslations.h"
#include "connectingdialog.h"
#include "desktopguifacade.h"
#include "discoverydialog.h"
#include "mainwindow.h"
#include "newgamedialog.h"
#include "options.h"
#include "promotiondialog.h"
#include "syncdialog.h"

DesktopGuiFacade::DesktopGuiFacade(const Options& options, QObject *parent)
    : GuiFacade{parent},
      m_mainWindow(new MainWindow),
      m_options(options)
{
    connect(m_mainWindow, &MainWindow::connectRequested, this, &GuiFacade::connectRequested);
    connect(m_mainWindow, &MainWindow::disconnectRequested, this, &GuiFacade::disconnectRequested);
    connect(m_mainWindow, &MainWindow::sendFenRequested, this, &GuiFacade::sendFenRequested);
    connect(m_mainWindow, &MainWindow::requestMove, this, &GuiFacade::requestMove);
    connect(m_mainWindow, &MainWindow::requestNewGame, this, &GuiFacade::showNewGameDialog);
    connect(m_mainWindow, &MainWindow::requestEdit, this, &GuiFacade::requestEdit);
    connect(m_mainWindow, &MainWindow::requestDraw, this, [this](){
        emit requestDraw(m_activeColour);
    });
    connect(m_mainWindow, &MainWindow::requestResignation, this, [this](){
        emit requestDraw(m_activeColour);
    });
    QMetaObject::invokeMethod(this, [this]() {
        setConnectionState(ConnectionState::Disconnected);
        setGameProgress(GameProgress(GameProgress::InProgress));
        setActiveColour(Chessboard::Colour::White);
    });
    updateStatusMessage();
    m_mainWindow->show();
}

DesktopGuiFacade::~DesktopGuiFacade()
{
    delete m_mainWindow;
}

void DesktopGuiFacade::showConnectingPopup(const QString& address)
{
    if (m_connectingDialog)
        delete m_connectingDialog;
    m_connectingDialog = new ConnectingDialog(address, m_mainWindow);
    connect(m_connectingDialog, &ConnectingDialog::rejected, this, [this]() {
       emit disconnectRequested();
    });
    m_connectingDialog->setModal(true);
    m_connectingDialog->show();
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);
    m_mainWindow->setStatusMessage(tr("Connecting to %1...").arg(address));
}

void DesktopGuiFacade::hideConnectingPopup()
{
    if (m_connectingDialog)
        m_connectingDialog->hide();
    QGuiApplication::restoreOverrideCursor();
}

void DesktopGuiFacade::showDiscoveryPopup()
{
    if (m_discoveryDialog)
        delete m_discoveryDialog;
    m_discoveryDialog = new DiscoveryDialog(m_mainWindow);
    connect(m_discoveryDialog, &DiscoveryDialog::accepted, this, [this]() {
        int index = m_discoveryDialog->getSelectedIndex();
        Q_ASSERT(index != -1);
        if (index != -1)
            emit connectToDiscoveredBoard(index);
    });
    connect(m_discoveryDialog, &DiscoveryDialog::finished, this, [this]() {
        emit cancelDiscovery();
    });
    connect(m_discoveryDialog, &DiscoveryDialog::refresh, this, &GuiFacade::restartDiscovery);
    m_discoveryDialog->setModal(true);
    m_discoveryDialog->show();
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);
}

void DesktopGuiFacade::addBoardToDiscoveryPopup(int index, const QString& address)
{
    Q_ASSERT(m_discoveryDialog);
    m_discoveryDialog->addItem(index, address);
}

void DesktopGuiFacade::setConnectionState(ConnectionState state)
{
    m_mainWindow->setConnectionState(state);
    m_connectionState = state;
    updateStatusMessage();
}

void DesktopGuiFacade::updateStatusMessage()
{
    if (m_progress.state == GameProgress::InProgress) {
        m_mainWindow->setStatusMessage(tr("%1 - %2 - %3", "status message").arg(
            connectionStateToStringTitle(m_connectionState),
            m_progress.toString(),
            tr("%1 to play").arg(colourToStringTitle(m_activeColour))));
    } else {
        m_mainWindow->setStatusMessage(tr("%1 - %2", "status message").arg(
            connectionStateToStringTitle(m_connectionState),
            m_progress.toString()));
    }
}

void DesktopGuiFacade::showNoBoardsDiscoveredPopup()
{
    Q_ASSERT(m_discoveryDialog);
    QMessageBox::warning(m_discoveryDialog, tr("Discovery Error"), tr("No board discovered."), QMessageBox::Ok);
}

void DesktopGuiFacade::discoveryFinished()
{
    Q_ASSERT(m_discoveryDialog);
    QGuiApplication::restoreOverrideCursor();
}

void DesktopGuiFacade::showErrorPopup(const QString& errorMessage)
{
    QMessageBox::critical(m_mainWindow, tr("Error"), errorMessage, QMessageBox::Ok);
}

void DesktopGuiFacade::clearDiscovered()
{
    Q_ASSERT(m_discoveryDialog);
    m_discoveryDialog->clear();
}

void DesktopGuiFacade::setBoardState(const Chessboard::BoardState& newState)
{
    m_mainWindow->setBoardState(newState);
}

void DesktopGuiFacade::showInvalidFenStringErrorPopup(const QString& fen)
{
    QMessageBox::critical(m_mainWindow, tr("Send FEN"), tr("FEN string is invalid:\n%1").arg(fen), QMessageBox::Ok);
}

void DesktopGuiFacade::showLocalNeedsSyncPopup()
{
    showSyncPopup(SyncDialog::LocalOutOfSyncWithRemote);
}

void DesktopGuiFacade::showRemoteNeedsSyncPopup()
{
    showSyncPopup(SyncDialog::RemoteOutOfSyncWithLocal);
}

void DesktopGuiFacade::showSyncPopup(SyncDialog::Direction direction)
{
    SyncDialog *syncDialog = new SyncDialog(m_mainWindow);
    syncDialog->setAttribute(Qt::WA_DeleteOnClose);
    syncDialog->setDirection(direction);
    connect(syncDialog, &SyncDialog::syncLocalToRemote, this, &GuiFacade::syncFromLocalToRemote);
    connect(syncDialog, &SyncDialog::syncRemoteToLocal, this, &GuiFacade::syncFromRemoteToLocal);
    syncDialog->setModal(true);
    syncDialog->show();
}

void DesktopGuiFacade::setGameProgress(const GameProgress& progress)
{
    m_progress = progress;
    m_mainWindow->setGameInProgress(progress.state == GameProgress::InProgress);
    updateStatusMessage();
}

void DesktopGuiFacade::showGameOverPopup(const GameProgress& progress)
{
    QMessageBox::information(m_mainWindow, tr("Game Over"), progress.toString(), QMessageBox::Ok);
}

void DesktopGuiFacade::setActiveColour(Chessboard::Colour colour)
{
    m_activeColour = colour;
    updateStatusMessage();
}

void DesktopGuiFacade::showPromotionPopup()
{
    PromotionDialog *promotionDialog = new PromotionDialog(m_mainWindow);
    promotionDialog->setModal(true);
    connect(promotionDialog, &PromotionDialog::pieceSelected, this,
            [this, promotionDialog](Chessboard::Piece piece){
        promotionDialog->deleteLater();
        QMetaObject::invokeMethod(this, [this, piece]() {
            emit requestPromotion(piece);
        }, Qt::QueuedConnection);
    });
    connect(promotionDialog, &PromotionDialog::rejected, this, [this, promotionDialog]() {
        promotionDialog->show();
    });
    promotionDialog->show();
}

void DesktopGuiFacade::showDrawRequestedPopup(Chessboard::Colour requestor)
{
    if (QMessageBox::question(m_mainWindow, tr("Draw Requested"),
                              tr("%1 requests a draw. Accept?").arg(colourToStringTitle(requestor)),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit requestDraw((requestor == Chessboard::Colour::White) ?
                         Chessboard::Colour::Black : Chessboard::Colour::White);
    }
}

void DesktopGuiFacade::showIllegalEditPopup(Chessboard::IllegalBoardReason reason)
{
    QString message;
    switch (reason) {
    case Chessboard::IllegalBoardReason::None:
        message = tr("None");
        break;
    case Chessboard::IllegalBoardReason::NoWhiteKing:
        message = tr("No white king");
        break;
    case Chessboard::IllegalBoardReason::NoBlackKing:
        message = tr("No white king");
        break;
    case Chessboard::IllegalBoardReason::MoreThanOneWhiteKing:
        message = tr("More than one white king");
        break;
    case Chessboard::IllegalBoardReason::MoreThanOneBlackKing:
        message = tr("More than one black king");
        break;
    case Chessboard::IllegalBoardReason::TooManyWhitePawns:
        message = tr("Too many white pawns");
        break;
    case Chessboard::IllegalBoardReason::TooManyBlackPawns:
        message = tr("Too many black pawns");
        break;
    case Chessboard::IllegalBoardReason::TooManyWhitePieces:
        message = tr("Too many white pieces");
        break;
    case Chessboard::IllegalBoardReason::TooManyBlackPieces:
        message = tr("Too many black pieces");
        break;
    case Chessboard::IllegalBoardReason::NonActivePlayerInCheck:
        message = tr("Non-active player in check");
        break;
    }
    QMessageBox::warning(m_mainWindow, tr("Illegal Edit"),
                         message);
}

void DesktopGuiFacade::setEditMode(bool enabled)
{
    m_mainWindow->setEditMode(enabled);
}

void DesktopGuiFacade::showNewGameDialog()
{
    NewGameDialog *newGameDialog = new NewGameDialog(m_mainWindow);
    newGameDialog->setAttribute(Qt::WA_DeleteOnClose);
    newGameDialog->setModal(true);
    newGameDialog->setGameOptions(m_gameOptions);
    newGameDialog->show();
    connect(newGameDialog, &NewGameDialog::newGameRequested, this,
            [this, newGameDialog](const Chessboard::GameOptions& gameOptions){
                QMetaObject::invokeMethod(this, [this, gameOptions]() {
                    emit requestNewGame(gameOptions);
                    }, Qt::QueuedConnection);
            });
}

void DesktopGuiFacade::gameOptionsChanged(const Chessboard::GameOptions& gameOptions)
{
    m_gameOptions = gameOptions;
    m_mainWindow->setLocalPlayer(Chessboard::Colour::White,
                                 gameOptions.white.playerType == Chessboard::PlayerType::Human &&
                                 gameOptions.white.playerLocation == Chessboard::PlayerLocation::LocalApp);
    m_mainWindow->setLocalPlayer(Chessboard::Colour::Black,
                                 gameOptions.black.playerType == Chessboard::PlayerType::Human &&
                                 gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalApp);
}

void DesktopGuiFacade::showBluetoothPermissionDeniedPopup()
{
    QMessageBox::warning(m_mainWindow, tr("Bluetooth Permission Denied"), tr("Bluetooth permission denied."));
}
