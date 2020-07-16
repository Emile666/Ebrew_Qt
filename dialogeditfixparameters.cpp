#include "dialogeditfixparameters.h"
#include "ui_dialogeditfixparameters.h"

dialogeditfixparameters::dialogeditfixparameters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialogeditfixparameters)
{
    ui->setupUi(this);
}

dialogeditfixparameters::~dialogeditfixparameters()
{
    delete ui;
}
