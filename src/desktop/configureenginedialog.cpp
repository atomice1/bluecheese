#include <QFileDialog>

#include "configureenginedialog.h"
#include "ui_configureenginedialog.h"

ConfigureEngineDialog::ConfigureEngineDialog(const QString& message,
                                             const QString& stockfishPath,
                                             QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureEngineDialog)
{
    ui->setupUi(this);
    ui->message->setText(QString(QLatin1String("Error: %1")).arg(message));
    ui->lineEdit->setText(stockfishPath);
    connect(this, &QDialog::accepted, [this]() {
        emit configureEngine(ui->lineEdit->text());
    });
}

ConfigureEngineDialog::~ConfigureEngineDialog()
{
    delete ui;
}

void ConfigureEngineDialog::on_pushButton_clicked()
{
#ifdef Q_OS_WIN
    QString filename = QLatin1String("stockfish.exe");
#else
    QString filename = QLatin1String("stockfish");
#endif
    QString stockfishPath = QFileDialog::getOpenFileName(this,
                                                         m_message,
                                                         /* dir */ QString(),
                                                         filename);
    ui->lineEdit->setText(stockfishPath);
}
