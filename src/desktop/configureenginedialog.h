#ifndef CONFIGUREENGINEDIALOG_H
#define CONFIGUREENGINEDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigureEngineDialog;
}

class ConfigureEngineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureEngineDialog(const QString& message,
                                   const QString& stockfishPath,
                                   QWidget *parent = nullptr);
    ~ConfigureEngineDialog();

signals:
    void configureEngine(const QString& stockfishPath);

private slots:
    void on_pushButton_clicked();

private:
    Ui::ConfigureEngineDialog *ui;
    QString m_message;
};

#endif // CONFIGUREENGINEDIALOG_H
