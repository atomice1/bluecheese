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

#include <QMessageBox>
#include "mainwindow.h"
#include "chessboardscene.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(new ChessboardScene(this))
{
    ui->setupUi(this);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->fitInView(ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);
    QFont font = ui->editToolBar->font();
    font.setPointSizeF(font.pointSizeF() * 2.0);
    ui->editToolBar->setFont(font);
    ui->editToolBar->hide();
    connect(m_scene, &ChessboardScene::requestMove, this, &MainWindow::requestMove);
    connect(m_scene, &ChessboardScene::squareSelected, this, &MainWindow::squareSelected);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Connect_triggered()
{
    emit connectRequested();
}

void MainWindow::setConnectionState(ConnectionState state)
{
    ui->action_Connect->setEnabled(state == ConnectionState::Disconnected);
    ui->action_Disconnect->setEnabled(state == ConnectionState::Connected);
}

void MainWindow::on_action_Disconnect_triggered()
{
    emit disconnectRequested();
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    ui->graphicsView->fitInView(ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LayoutRequest: {
        bool ret = QMainWindow::event(event);
        ui->graphicsView->fitInView(ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);
        return ret;
    }
    default:
        return QMainWindow::event(event);
    }
}

void MainWindow::setBoardState(const Chessboard::BoardState& newState)
{
    qDebug("MainWindow::setBoardState");
    m_scene->setBoardState(newState);
    ui->fenLineEdit->setText(newState.toFenString());
    updateEditToolbarState();
}

void MainWindow::setStatusMessage(const QString& message)
{
    if (isInEditMode())
        ui->statusbar->showMessage(tr("%0 - Edit Mode").arg(message));
    else
        ui->statusbar->showMessage(message);
}

void MainWindow::on_fenLineEdit_returnPressed()
{
    emit sendFenRequested(ui->fenLineEdit->text());
}

void MainWindow::on_sendPushButton_clicked()
{
    emit sendFenRequested(ui->fenLineEdit->text());
}

void MainWindow::on_action_New_Game_triggered()
{
    emit requestNewGame();
}

void MainWindow::on_action_Request_Draw_triggered()
{
    emit requestDraw();
}

void MainWindow::on_action_Resign_triggered()
{
    emit requestResignation();
}

void MainWindow::setGameInProgress(bool flag)
{
    ui->action_Request_Draw->setEnabled(flag);
    ui->action_Resign->setEnabled(flag);
    m_scene->setGameInProgress(flag);
}

void MainWindow::on_action_Exit_triggered()
{
    qApp->quit();
}

void MainWindow::on_editToolBar_visibilityChanged(bool visible)
{
    if (visible)
        setEditMode(visible);
    else
        emit requestEdit(m_scene->boardState());
}

void MainWindow::setEditMode(bool enabled)
{
    ui->editToolBar->setVisible(enabled);
    ui->action_Edit_Mode->setChecked(enabled);
    m_scene->setEditMode(enabled);
}

bool MainWindow::isInEditMode() const
{
    return ui->editToolBar->isVisible();
}

void MainWindow::squareSelected()
{
    updateEditToolbarState();
}

void MainWindow::updateEditToolbarState()
{
    Chessboard::BoardState state = m_scene->boardState();
    Chessboard::Square square = m_scene->selectedSquare();
    Chessboard::ColouredPiece piece =
        square.isValid() ? state[square] : Chessboard::ColouredPiece::None;
    ui->action_Empty->setChecked(square.isValid() &&
                                 piece == Chessboard::ColouredPiece::None);
    ui->action_White_Pawn->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::WhitePawn);
    ui->action_White_Rook->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::WhiteRook);
    ui->action_White_Bishop->setChecked(square.isValid() &&
                                        piece == Chessboard::ColouredPiece::WhiteBishop);
    ui->action_White_Knight->setChecked(square.isValid() &&
                                        piece == Chessboard::ColouredPiece::WhiteKnight);
    ui->action_White_Queen->setChecked(square.isValid() &&
                                       piece == Chessboard::ColouredPiece::WhiteQueen);
    ui->action_White_King->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::WhiteKing);
    ui->action_Black_Pawn->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::BlackPawn);
    ui->action_Black_Rook->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::BlackRook);
    ui->action_Black_Bishop->setChecked(square.isValid() &&
                                        piece == Chessboard::ColouredPiece::BlackBishop);
    ui->action_Black_Knight->setChecked(square.isValid() &&
                                        piece == Chessboard::ColouredPiece::BlackKnight);
    ui->action_Black_Queen->setChecked(square.isValid() &&
                                      piece == Chessboard::ColouredPiece::BlackQueen);
    ui->action_Black_King->setChecked(square.isValid() &&
                                     piece == Chessboard::ColouredPiece::BlackKing);
}

void MainWindow::changePiece(Chessboard::ColouredPiece piece)
{
    Chessboard::Square square = m_scene->selectedSquare();
    if (square.isValid()) {
        Chessboard::BoardState state = m_scene->boardState();
        state[square] = piece;
        setBoardState(state);
    }
    updateEditToolbarState();
}

void MainWindow::on_action_White_Pawn_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhitePawn);
}

void MainWindow::on_action_White_Knight_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhiteKnight);
}

void MainWindow::on_action_White_Rook_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhiteRook);
}

void MainWindow::on_action_White_Bishop_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhiteBishop);
}

void MainWindow::on_action_White_Queen_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhiteQueen);
}

void MainWindow::on_action_White_King_triggered()
{
    changePiece(Chessboard::ColouredPiece::WhiteKing);
}

void MainWindow::on_action_Black_Pawn_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackPawn);
}

void MainWindow::on_action_Black_Knight_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackKnight);
}

void MainWindow::on_action_Black_Rook_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackRook);
}

void MainWindow::on_action_Black_Bishop_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackBishop);
}

void MainWindow::on_action_Black_Queen_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackQueen);
}

void MainWindow::on_action_Black_King_triggered()
{
    changePiece(Chessboard::ColouredPiece::BlackKing);
}

void MainWindow::on_action_Empty_triggered()
{
    changePiece(Chessboard::ColouredPiece::None);
}
