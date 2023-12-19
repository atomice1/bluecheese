/*
 * bluecheese
 * Copyright (C) 2023 Chris January
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

#include "newgamedialog.h"
#include "ui_newgamedialog.h"

NewGameDialog::NewGameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewGameDialog)
{
    ui->setupUi(this);
    connect(this, &QDialog::accepted, this, [this]() {
        Chessboard::GameOptions gameOptions;
        gameOptions.white.playerType =
            ui->whiteAiRadioButton->isChecked() ? Chessboard::PlayerType::Ai : Chessboard::PlayerType::Human;
        gameOptions.black.playerType =
            ui->blackAiRadioButton->isChecked() ? Chessboard::PlayerType::Ai : Chessboard::PlayerType::Human;
        if (gameOptions.white.playerType == Chessboard::PlayerType::Ai)
            gameOptions.white.playerLocation = Chessboard::PlayerLocation::LocalApp;
        else
            gameOptions.white.playerLocation =
                ui->whiteOnBoardRadioButton->isChecked() ? Chessboard::PlayerLocation::LocalBoard : Chessboard::PlayerLocation::LocalApp;
        if (gameOptions.black.playerType == Chessboard::PlayerType::Ai)
            gameOptions.black.playerLocation = Chessboard::PlayerLocation::LocalApp;
        else
            gameOptions.black.playerLocation =
                ui->blackOnBoardRadioButton->isChecked() ? Chessboard::PlayerLocation::LocalBoard : Chessboard::PlayerLocation::LocalApp;
        gameOptions.white.aiNominalElo = ui->whiteAiDifficultySlider->value();
        gameOptions.black.aiNominalElo = ui->blackAiDifficultySlider->value();
        emit newGameRequested(gameOptions);
    });
}

NewGameDialog::~NewGameDialog()
{
    delete ui;
}
