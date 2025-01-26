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
    void setEditMode(bool enabled);
    void setLocalPlayer(Chessboard::Colour color, bool localPlayer);
    void setAssistance(const QList<Chessboard::AssistanceColour>& colours);
    void setCanUndo(bool enabled);

signals:
    void connectRequested();
    void disconnectRequested();
    void sendFenRequested(const QString& fen);
    void requestMove(int fromRow, int fromCol, int toRow, int toCol);
    void requestNewGame();
    void requestDraw();
    void requestResignation();
    void requestEdit(const Chessboard::BoardState& state);
    void requestUndo();

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
    void on_editToolBar_visibilityChanged(bool visible);
    void squareSelected();
    void updateEditToolbarState();
    void changePiece(Chessboard::ColouredPiece piece);
    void on_action_White_Pawn_triggered();
    void on_action_White_Knight_triggered();
    void on_action_White_Bishop_triggered();
    void on_action_White_Rook_triggered();
    void on_action_White_Queen_triggered();
    void on_action_White_King_triggered();
    void on_action_Black_Pawn_triggered();
    void on_action_Black_Knight_triggered();
    void on_action_Black_Bishop_triggered();
    void on_action_Black_Rook_triggered();
    void on_action_Black_Queen_triggered();
    void on_action_Black_King_triggered();
    void on_action_Empty_triggered();

private:
    bool isInEditMode() const;

    Ui::MainWindow *ui;
    ChessboardScene *m_scene {};
};

#endif // MAINWINDOW_H
