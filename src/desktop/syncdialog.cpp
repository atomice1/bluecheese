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

#include "syncdialog.h"
#include "ui_syncdialog.h"

SyncDialog::SyncDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SyncDialog)
{
    ui->setupUi(this);
    connect(ui->cancelPushButton, &QPushButton::clicked, this, &SyncDialog::rejected);
}

SyncDialog::~SyncDialog()
{
    delete ui;
}

void SyncDialog::setDirection(SyncDialog::Direction direction)
{
}

void SyncDialog::on_localToRemotePushButton_clicked()
{
    emit syncLocalToRemote();
    accept();
}

void SyncDialog::on_remoteToLocalPushButton_clicked()
{
    emit syncRemoteToLocal();
    accept();
}

