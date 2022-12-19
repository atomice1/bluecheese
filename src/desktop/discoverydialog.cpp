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

#include "discoverydialog.h"
#include "ui_discoverydialog.h"

DiscoveryDialog::DiscoveryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiscoveryDialog)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &DiscoveryDialog::refresh);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

DiscoveryDialog::~DiscoveryDialog()
{
    delete ui;
}

void DiscoveryDialog::addItem(int index, const QString& address)
{
    if (index == 0)
        ui->listWidget->clear();
    ui->listWidget->addItem(address);
}

int DiscoveryDialog::getSelectedIndex() const
{
    return ui->listWidget->currentRow();
}

void DiscoveryDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentRow != -1);
}

void DiscoveryDialog::clear()
{
    ui->listWidget->clear();
}
