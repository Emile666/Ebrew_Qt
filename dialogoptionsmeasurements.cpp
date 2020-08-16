/**************************************************************************************
** Filename    : dialogoptionsmeasurements.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen
**               with which to change parameters for temperature and flow measurements.
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
#include "dialogoptionsmeasurements.h"
#include "ui_dialogoptionsmeasurements.h"

DialogOptionsMeasurements::DialogOptionsMeasurements(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsMeasurements)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);
    // Temperature Measurements
    ui->sbThltCal->setValue(pEbrew->RegEbrew->value("THLT_OFFSET").toDouble());
    ui->sbTmltCal->setValue(pEbrew->RegEbrew->value("TMLT_OFFSET").toDouble());
    ui->sbTboilCal->setValue(pEbrew->RegEbrew->value("TBOIL_OFFSET").toDouble());
    ui->sbTcfcCal->setValue(pEbrew->RegEbrew->value("TCFC_OFFSET").toDouble());
    // Flow Measurements
    ui->sbFlow1->setValue(pEbrew->RegEbrew->value("FLOW1_ERR").toDouble());
    ui->sbFlow2->setValue(pEbrew->RegEbrew->value("FLOW2_ERR").toDouble());
    ui->sbFlow3->setValue(pEbrew->RegEbrew->value("FLOW3_ERR").toDouble());
    ui->sbFlow4->setValue(pEbrew->RegEbrew->value("FLOW4_ERR").toDouble());
    ui->cbTcomp->setChecked(pEbrew->RegEbrew->value("FLOW_TEMP_CORR").toInt() == 1);
    ui->sbMltEmpty->setValue(pEbrew->RegEbrew->value("MIN_FR_MLT_PERC").toDouble());
    ui->sbBkEmpty->setValue(pEbrew->RegEbrew->value("MIN_FR_BOIL_PERC").toDouble());
}

DialogOptionsMeasurements::~DialogOptionsMeasurements()
{
    delete ui;
}

void DialogOptionsMeasurements::on_buttonBox_accepted()
{
    // Temperature Measurements
    pEbrew->RegEbrew->setValue("THLT_OFFSET",ui->sbThltCal->value());
    pEbrew->RegEbrew->setValue("TMLT_OFFSET",ui->sbTmltCal->value());
    pEbrew->RegEbrew->setValue("TBOIL_OFFSET",ui->sbTboilCal->value());
    pEbrew->RegEbrew->setValue("TCFC_OFFSET",ui->sbTcfcCal->value());
    // Flow Measurements
    pEbrew->RegEbrew->setValue("FLOW1_ERR",ui->sbFlow1->value());
    pEbrew->RegEbrew->setValue("FLOW2_ERR",ui->sbFlow2->value());
    pEbrew->RegEbrew->setValue("FLOW3_ERR",ui->sbFlow3->value());
    pEbrew->RegEbrew->setValue("FLOW4_ERR",ui->sbFlow4->value());
    pEbrew->RegEbrew->setValue("FLOW_TEMP_CORR",ui->cbTcomp->isChecked() ? 1 : 0);
    pEbrew->RegEbrew->setValue("MIN_FR_MLT_PERC",ui->sbMltEmpty->value());
    pEbrew->RegEbrew->setValue("MIN_FR_BOIL_PERC",ui->sbBkEmpty->value());
} // DialogOptionsMeasurements::on_buttonBox_accepted()
