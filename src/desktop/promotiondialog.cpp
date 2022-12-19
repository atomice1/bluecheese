#include "promotiondialog.h"
#include "ui_promotiondialog.h"

PromotionDialog::PromotionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PromotionDialog)
{
    ui->setupUi(this);
}

PromotionDialog::~PromotionDialog()
{
    delete ui;
}

void PromotionDialog::on_pushButton_bishop_clicked()
{
    emit pieceSelected(Chessboard::Piece::Bishop);
}

void PromotionDialog::on_pushButton_rook_clicked()
{
    emit pieceSelected(Chessboard::Piece::Rook);
}

void PromotionDialog::on_pushButton_knight_clicked()
{
    emit pieceSelected(Chessboard::Piece::Knight);
}

void PromotionDialog::on_pushButton_queen_clicked()
{
    emit pieceSelected(Chessboard::Piece::Queen);
}
