#ifndef PROMOTIONDIALOG_H
#define PROMOTIONDIALOG_H

#include <QDialog>
#include "chessboard.h"

namespace Ui {
class PromotionDialog;
}

class PromotionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PromotionDialog(QWidget *parent = nullptr);
    ~PromotionDialog();

signals:
    void pieceSelected(Chessboard::Piece piece);

private slots:
    void on_pushButton_bishop_clicked();
    void on_pushButton_rook_clicked();
    void on_pushButton_knight_clicked();
    void on_pushButton_queen_clicked();

private:
    Ui::PromotionDialog *ui;
};

#endif // PROMOTIONDIALOG_H
