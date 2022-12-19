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
    connect(m_scene, &ChessboardScene::requestMove, this, &MainWindow::requestMove);
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
}

void MainWindow::setStatusMessage(const QString& message)
{
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
