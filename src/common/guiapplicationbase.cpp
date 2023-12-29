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
#include <QBluetoothPermission>
#include "applicationfacade.h"
#include "commontranslations.h"
#include "guiapplicationbase.h"
#include "guifacade.h"

using namespace Chessboard;

GuiApplicationBase::GuiApplicationBase(GuiFacade *guiFacade_, const Options &options, QObject *parent) :
    ApplicationBase(options, parent),
    m_guiFacade(guiFacade_)
{
    connect(facade(), &ApplicationFacade::connected, this, &GuiApplicationBase::onConnected);
    connect(facade(), &ApplicationFacade::disconnected, this, &GuiApplicationBase::onDisconnected);
    connect(facade(), &ApplicationFacade::error, this, &GuiApplicationBase::onError);
    connect(facade(), &ApplicationFacade::discoveryFinished, this, &GuiApplicationBase::onDiscoveryFinished);
    connect(facade(), &ApplicationFacade::discoveryStarted, this, &GuiApplicationBase::onDiscoveryStarted);
    connect(facade(), &ApplicationFacade::boardDiscovered, this, &GuiApplicationBase::onBoardDiscovered);
    connect(facade(), &ApplicationFacade::connecting, this, &GuiApplicationBase::onConnecting);
    connect(facade(), &ApplicationFacade::noBoardsDiscovered, this, &GuiApplicationBase::onNoBoardsDiscovered);
    connect(facade(), &ApplicationFacade::connectionFailed, this, &GuiApplicationBase::onConnectionFailed);
    connect(facade(), &ApplicationFacade::boardStateChanged, this, &GuiApplicationBase::onBoardStateChanged);
    connect(facade(), &ApplicationFacade::invalidFenString, this, &GuiApplicationBase::onInvalidFenString);
    connect(facade(), &ApplicationFacade::drawRequested, this, &GuiApplicationBase::onDrawRequested);
    connect(facade(), &ApplicationFacade::gameProgressChanged, this, &GuiApplicationBase::onGameProgressChanged);
    connect(facade(), &ApplicationFacade::activeColourChanged, this, &GuiApplicationBase::onActiveColourChanged);
    connect(facade(), &ApplicationFacade::promotionRequired, this, &GuiApplicationBase::onPromotionRequired);
    connect(facade(), &ApplicationFacade::gameOptionsChanged, guiFacade(), &GuiFacade::gameOptionsChanged);
    connect(facade(), &ApplicationFacade::assistance, guiFacade(), &GuiFacade::assistance);
    connect(guiFacade(), &GuiFacade::connectRequested, this, &GuiApplicationBase::onConnectRequested);
    connect(guiFacade(), &GuiFacade::disconnectRequested, this, &GuiApplicationBase::onDisconnectRequested);
    connect(guiFacade(), &GuiFacade::cancelConnect, this, &GuiApplicationBase::onCancelConnect);
    connect(guiFacade(), &GuiFacade::restartDiscovery, this, &GuiApplicationBase::onRestartDiscovery);
    connect(guiFacade(), &GuiFacade::cancelDiscovery, this, &GuiApplicationBase::onCancelDiscovery);
    connect(guiFacade(), &GuiFacade::connectToDiscoveredBoard, this, &GuiApplicationBase::onConnectToDiscoveredBoard);
    connect(guiFacade(), &GuiFacade::sendFenRequested, this, &GuiApplicationBase::onSendFenRequested);
    connect(guiFacade(), &GuiFacade::syncFromRemoteToLocal, this, &GuiApplicationBase::onSyncFromRemoteToLocal);
    connect(guiFacade(), &GuiFacade::syncFromLocalToRemote, this, &GuiApplicationBase::onSyncFromLocalToRemote);
    connect(guiFacade(), &GuiFacade::requestMove, this, &GuiApplicationBase::onRequestMove);
    connect(guiFacade(), &GuiFacade::requestNewGame, this, &GuiApplicationBase::onRequestNewGame);
    connect(guiFacade(), &GuiFacade::requestDraw, this, &GuiApplicationBase::onRequestDraw);
    connect(guiFacade(), &GuiFacade::requestResignation, this, &GuiApplicationBase::onRequestResignation);
    connect(guiFacade(), &GuiFacade::requestPromotion, this, &GuiApplicationBase::onRequestPromotion);
    connect(guiFacade(), &GuiFacade::requestEdit, this, &GuiApplicationBase::onRequestEdit);
    guiFacade()->setConnectionState(ConnectionState::Disconnected);
    guiFacade()->setBoardState(BoardState::newGame());
    guiFacade()->gameOptionsChanged(facade()->gameOptions());
    QMetaObject::invokeMethod(this, &GuiApplicationBase::autoConnect, Qt::QueuedConnection);
}

GuiApplicationBase::~GuiApplicationBase()
{
    delete m_guiFacade;
}

void GuiApplicationBase::onConnected(Chessboard::RemoteBoard *board)
{
    qDebug("GuiApplicationBase::onConnect(%s)", qPrintable(board->address().toString()));
    m_connected = true;
    guiFacade()->setConnectionState(ConnectionState::Connected);
    guiFacade()->hideConnectingPopup();
    maybeShowNewGameDialog();
}

void GuiApplicationBase::onDisconnected()
{
    qDebug("GuiApplicationBase::onDisconnected()");
    guiFacade()->setConnectionState(ConnectionState::Disconnected);
    guiFacade()->hideConnectingPopup();
}

void GuiApplicationBase::onError(const QString& errorMessage)
{
    qDebug("GuiApplicationBase::onError(%s)", qPrintable(errorMessage));
    guiFacade()->showErrorPopup(errorMessage);
}

void GuiApplicationBase::onDiscoveryStarted()
{
    qDebug("GuiApplicationBase::onDiscoveryStarted");
    m_discovered.clear();
    guiFacade()->showDiscoveryPopup();
}

void GuiApplicationBase::onDiscoveryFinished()
{
    qDebug("GuiApplicationBase::onDiscoveryFinished");
    guiFacade()->discoveryFinished();
}

void GuiApplicationBase::onBoardDiscovered(const Chessboard::BoardAddress& address)
{
    m_discovered.append(address);
    guiFacade()->addBoardToDiscoveryPopup(m_discovered.size() - 1, address.toString());
}

void GuiApplicationBase::onConnecting(const Chessboard::BoardAddress& address)
{
    qDebug("GuiApplicationBase::onConnecting(%s)", qPrintable(address.toString()));
    guiFacade()->setConnectionState(ConnectionState::Connecting);
    guiFacade()->showConnectingPopup(address.toString());
}

void GuiApplicationBase::onNoBoardsDiscovered()
{
    qDebug("GuiApplicationBase::onNoBoardsDiscovered()");
    guiFacade()->showNoBoardsDiscoveredPopup();
}

void GuiApplicationBase::onConnectionFailed()
{
    qDebug("GuiApplicationBase::onConnectionFailed()");
    guiFacade()->setConnectionState(ConnectionState::Disconnected);
    guiFacade()->hideConnectingPopup();
}

void GuiApplicationBase::onConnectRequested()
{
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);
    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this,
                                &GuiApplicationBase::autoConnect);
        break;
    case Qt::PermissionStatus::Denied:
        guiFacade()->showBluetoothPermissionDeniedPopup();
        break;
    case Qt::PermissionStatus::Granted:
        facade()->startDiscovery();
        break;
    }
}

void GuiApplicationBase::autoConnect()
{
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);
    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this,
                                &GuiApplicationBase::autoConnect);
        break;
    case Qt::PermissionStatus::Denied:
        guiFacade()->showBluetoothPermissionDeniedPopup();
        break;
    case Qt::PermissionStatus::Granted:
        facade()->connectToLastOrDiscover();
        break;
    }
}

void GuiApplicationBase::onDisconnectRequested()
{
    facade()->disconnectFromBoard();
}

void GuiApplicationBase::onCancelConnect()
{
    facade()->disconnectFromBoard();
    maybeShowNewGameDialog();
}

void GuiApplicationBase::onRestartDiscovery()
{
    guiFacade()->clearDiscovered();
    facade()->stopDiscovery();
    facade()->startDiscovery();
}

void GuiApplicationBase::onCancelDiscovery()
{
    facade()->stopDiscovery();
    maybeShowNewGameDialog();
}

void GuiApplicationBase::onConnectToDiscoveredBoard(int index)
{
    qDebug("GuiApplicationBase::onConnectToDiscoveredBoard");
    Q_ASSERT(index >= 0 && index < m_discovered.size());
    if (index < 0 || index >= m_discovered.size())
        return;
    facade()->connectToBoard(m_discovered.at(index));
}

void GuiApplicationBase::onBoardStateChanged(const Chessboard::BoardState& newState)
{
    qDebug("GuiApplicationBase::onBoardStateChanged");
    guiFacade()->setBoardState(newState);
}

void GuiApplicationBase::onSendFenRequested(const QString& fen)
{
    qDebug("GuiApplicationBase::onSendFenRequested(%s)", qPrintable(fen));
    facade()->setBoardStateFromFen(fen);
}

void GuiApplicationBase::onInvalidFenString(const QString& fen)
{
    qDebug("GuiApplicationBase::onInvalidFenString(%s)", qPrintable(fen));
    guiFacade()->showInvalidFenStringErrorPopup(fen);
}

void GuiApplicationBase::onSyncFromLocalToRemote()
{
    qDebug("GuiApplicationBase::onSyncFromLocalToRemote()");
    facade()->requestRemoteBoardState();
}

void GuiApplicationBase::onSyncFromRemoteToLocal()
{
    qDebug("GuiApplicationBase::onSyncFromRemoteToLocal()");
    facade()->sendLocalBoardState();
}

void GuiApplicationBase::onLocalOutOfSyncWithRemote()
{
    qDebug("GuiApplicationBase::onLocalOutOfSyncWithRemote()");
    guiFacade()->showLocalNeedsSyncPopup();
}

void GuiApplicationBase::onRemoteOutOfSyncWithLocal()
{
    qDebug("GuiApplicationBase::onRemoteOutOfSyncWithLocal()");
    guiFacade()->showRemoteNeedsSyncPopup();
}

void GuiApplicationBase::onRequestMove(int fromRow, int fromCol, int toRow, int toCol)
{
    qDebug("GuiApplicationBase::onRequestMove(%d, %d, %d, %d)", fromRow, fromCol, toRow, toCol);
    facade()->requestMove(fromRow, fromCol, toRow, toCol);
}

void GuiApplicationBase::onRequestNewGame(const Chessboard::GameOptions& gameOptions)
{
    qDebug("GuiApplicationBase::onRequestNewGame()");
    if ((gameOptions.white.playerLocation == Chessboard::PlayerLocation::LocalBoard ||
         gameOptions.black.playerLocation == Chessboard::PlayerLocation::LocalBoard) &&
        !m_connected)
        facade()->connectToLastOrDiscover();
    facade()->requestNewGame(gameOptions);
}

void GuiApplicationBase::onDrawRequested(Chessboard::Colour requestor)
{
    qDebug("GuiApplicationBase::onDrawRequested(%s)",
           (requestor == Chessboard::Colour::White) ? "white" : "black");
    guiFacade()->showDrawRequestedPopup(requestor);
}

void GuiApplicationBase::onActiveColourChanged(Chessboard::Colour colour)
{
    qDebug("GuiApplicationBase::onActiveColourChanged(%s)",
           (colour == Chessboard::Colour::White) ? "white" : "black");
    guiFacade()->setActiveColour(colour);
}

void GuiApplicationBase::onGameProgressChanged(const GameProgress& progress)
{
    qDebug("GuiApplicationBase::onGameProgressChanged(%s)",
           qPrintable(progress.toString()));
    guiFacade()->setGameProgress(progress);
    if (progress.state != GameProgress::InProgress)
        guiFacade()->showGameOverPopup(progress);
}

void GuiApplicationBase::onRequestDraw(Chessboard::Colour requestor)
{
    qDebug("GuiApplicationBase::onRequestDraw(%s)",
           (requestor == Chessboard::Colour::White) ? "white" : "black");
    facade()->requestDraw(requestor);
}

void GuiApplicationBase::onRequestResignation(Chessboard::Colour requestor)
{
    qDebug("GuiApplicationBase::onRequestResignation(%s)",
           (requestor == Chessboard::Colour::White) ? "white" : "black");
    facade()->requestResignation(requestor);
}

void GuiApplicationBase::onPromotionRequired()
{
    qDebug("GuiApplicationBase::onPromotionRequired()");
    guiFacade()->showPromotionPopup();
}

void GuiApplicationBase::onRequestPromotion(Chessboard::Piece piece)
{
    qDebug("GuiApplicationBase::onRequestPromotion(%s)",
           qPrintable(pieceToString(piece)));
    facade()->requestPromotion(piece);
}

void GuiApplicationBase::onRequestEdit(const Chessboard::BoardState& state)
{
    qDebug("GuiApplicationBase::onRequestEdit(...)");
    IllegalBoardReason reason = IllegalBoardReason::None;
    bool legal = state.isLegal(&reason);
    if (!legal) {
        guiFacade()->setEditMode(true);
        guiFacade()->showIllegalEditPopup(reason);
    } else {
        guiFacade()->setBoardState(state);
        guiFacade()->setEditMode(false);
        facade()->setBoardStateFromFen(state.toFenString());
    }
}

void GuiApplicationBase::maybeShowNewGameDialog()
{
    if (m_pendingNewGame) {
        m_pendingNewGame = false;
        QMetaObject::invokeMethod(guiFacade(), &GuiFacade::showNewGameDialog, Qt::QueuedConnection);
    }
}
