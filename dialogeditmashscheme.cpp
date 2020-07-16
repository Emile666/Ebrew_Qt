#include "dialogeditmashscheme.h"
#include "ui_dialogeditmashscheme.h"

DialogEditMashScheme::DialogEditMashScheme(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditMashScheme)
{
    ui->setupUi(this);
}

DialogEditMashScheme::~DialogEditMashScheme()
{
    delete ui;
}
