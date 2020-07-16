#include "dialogbrewdaysettings.h"
#include "ui_dialogbrewdaysettings.h"

DialogBrewDaySettings::DialogBrewDaySettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBrewDaySettings)
{
    ui->setupUi(this);
}

DialogBrewDaySettings::~DialogBrewDaySettings()
{
    delete ui;
}
