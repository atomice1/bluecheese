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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chessboard.h"
#include "connectionstate.h"

namespace Ui {
class MainWindow;
}

class ChessboardScene;
class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setConnectionState(ConnectionState state);
    void setBoardState(const Chessboard::BoardState& newState);
    void setStatusMessage(const QString& message);
    void setGameInProgress(bool flag);

signals:
    void connectRequested();
    void disconnectRequested();
    void sendFenRequested(const QString& fen);
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestNewGame();
    void requestDraw();
    void requestResignation();

protected:
    void resizeEvent(QResizeEvent *) override;
    bool event(QEvent *) override;

private slots:
    void on_action_Connect_triggered();
    void on_action_Disconnect_triggered();
    void on_fenLineEdit_returnPressed();
    void on_sendPushButton_clicked();
    void on_action_New_Game_triggered();
    void on_action_Request_Draw_triggered();
    void on_action_Resign_triggered();

    void on_action_Exit_triggered();

private:
    Ui::MainWindow *ui;
    ChessboardScene *m_scene;
};

#endif // MAINWINDOW_H
