#include "dialogoptionspidsettings.h"
#include "ui_dialogoptionspidsettings.h"

DialogOptionsPidSettings::DialogOptionsPidSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsPidSettings)
{
    ui->setupUi(this);
}

DialogOptionsPidSettings::~DialogOptionsPidSettings()
{
    delete ui;
}
