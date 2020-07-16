#include "dialogoptionsmeasurements.h"
#include "ui_dialogoptionsmeasurements.h"

DialogOptionsMeasurements::DialogOptionsMeasurements(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsMeasurements)
{
    ui->setupUi(this);
}

DialogOptionsMeasurements::~DialogOptionsMeasurements()
{
    delete ui;
}
