#include "dialogoptionspidsettings.h"
#include "ui_dialogoptionspidsettings.h"

DialogOptionsPidSettings::DialogOptionsPidSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsPidSettings)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);
    ui->sbTs->setValue(pEbrew->RegEbrew->value("Ts").toInt());
    ui->sbKc->setValue(pEbrew->RegEbrew->value("Kc").toInt());
    ui->sbTi->setValue(pEbrew->RegEbrew->value("Ti").toInt());
    ui->sbTd->setValue(pEbrew->RegEbrew->value("Td").toInt());
    ui->sbSlpL->setValue(pEbrew->RegEbrew->value("TSET_SLOPE_LIM").toInt());
    ui->cbDStart->setChecked(pEbrew->delayed_start);
    if (pEbrew->delayed_start)
    {
        ui->dtDstart->setDateTime(pEbrew->dlyStartTime);
    } // if
    else
    {
        ui->dtDstart->setMinimumDate(QDate::currentDate());
        ui->dtDstart->setMaximumDate(QDate::currentDate().addDays(5));
    } // else
}

DialogOptionsPidSettings::~DialogOptionsPidSettings()
{
    delete ui;
}

void DialogOptionsPidSettings::on_buttonBox_accepted()
{
    pEbrew->RegEbrew->setValue("Ts",ui->sbTs->value());
    pEbrew->RegEbrew->setValue("Kc",ui->sbKc->value());
    pEbrew->RegEbrew->setValue("Ti",ui->sbTi->value());
    pEbrew->RegEbrew->setValue("Td",ui->sbTd->value());
    pEbrew->RegEbrew->setValue("TSET_SLOPE_LIM",ui->sbSlpL->value());
    if (ui->cbDStart->isChecked())
    {
        pEbrew->delayed_start = true;
        pEbrew->dlyStartTime  = ui->dtDstart->dateTime();
    }
    else
    {
        pEbrew->delayed_start = false;
    }
} // DialogOptionsPidSettings::on_buttonBox_accepted()

void DialogOptionsPidSettings::on_cbDStart_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->lblDstart->setEnabled(true);
        ui->dtDstart->setEnabled(true);
    } // if
    else
    {
        ui->lblDstart->setEnabled(false);
        ui->dtDstart->setEnabled(false);
    } // else
} // DialogOptionsPidSettings::on_cbDStart_stateChanged(
