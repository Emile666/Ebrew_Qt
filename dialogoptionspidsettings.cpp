/**************************************************************************************
** Filename    : dialogoptionspidsettings.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen
**               with which to change parameters for the PID-controller.
** License     : This is free software: you can redistribute it and/or modify
**               it under the terms of the GNU General Public License as published by
**               the Free Software Foundation, either version 3 of the License, or
**               (at your option) any later version.
**
**               This file is distributed in the hope that it will be useful,
**               but WITHOUT ANY WARRANTY; without even the implied warranty of
**               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**               GNU General Public License for more details.
**
**               You should have received a copy of the GNU General Public License
**               along with this file.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
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
    ui->cbDStart->setChecked(pEbrew->delayedStart);
    if (pEbrew->delayedStart)
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
    pEbrew->PidCtrlHlt->pidInit(ui->sbKc->value(),ui->sbTi->value(),ui->sbTd->value(),ui->sbTs->value());
    pEbrew->PidCtrlBk->pidInit(ui->sbKc->value(),ui->sbTi->value(),ui->sbTd->value(),ui->sbTs->value());
    // Now update schedule task for PID-controller with new task-time
    pEbrew->schedulerEbrew->set_task_time_period((uint16_t)(1000 * ui->sbTs->value()),"pidControl");
    if (ui->cbDStart->isChecked())
    {
        pEbrew->delayedStart = true;
        pEbrew->dlyStartTime  = ui->dtDstart->dateTime();
    }
    else
    {
        pEbrew->delayedStart = false;
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
