/**************************************************************************************
** Filename    : dialogbrewdaysettings.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen for changing mashing, sparging,
**               boiling and CIP parameters.
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
#include "dialogbrewdaysettings.h"
#include "ui_dialogbrewdaysettings.h"

DialogBrewDaySettings::DialogBrewDaySettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBrewDaySettings)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);

    //-------------------------------
    // Mashing
    //-------------------------------
    ui->sbMash1->setValue(pEbrew->RegEbrew->value("TOffset0").toDouble());
    ui->sbMash2->setValue(pEbrew->RegEbrew->value("TOffset").toDouble());
    ui->sbMash3->setValue(pEbrew->RegEbrew->value("TOffset2").toDouble());
    ui->sbMash4->setValue((pEbrew->RegEbrew->value("HLT_Bcap").toDouble()));
    ui->sbMash5->setValue((pEbrew->RegEbrew->value("PREHEAT_TIME").toDouble()));
    if (pEbrew->RegEbrew->value("CB_dpht").toInt())
    {   // Dynamic preheat time
        ui->cbMash1->setChecked(true);
        ui->lblMash4->setEnabled(true);
        ui->sbMash4->setEnabled(true);
        ui->lblMash5->setEnabled(false);
        ui->sbMash5->setEnabled(false);
    } // if
    else
    {   // fixed time for preheating
        ui->cbMash1->setChecked(false);
        ui->lblMash4->setEnabled(false);
        ui->sbMash4->setEnabled(false);
        ui->lblMash5->setEnabled(true);
        ui->sbMash5->setEnabled(true);
    } // else
    ui->cbMash2->setChecked(pEbrew->RegEbrew->value("CB_Malt_First").toInt());
    ui->cbMash3->setChecked(pEbrew->RegEbrew->value("CB_Mash_Rest").toInt());
    ui->cbMash4->setChecked(pEbrew->RegEbrew->value("CB_pumps_on").toInt());

    //-------------------------------
    // Sparging
    //-------------------------------
    ui->sbSparge1->setValue(pEbrew->RegEbrew->value("SP_BATCHES").toInt());
    ui->sbSparge2->setValue(pEbrew->RegEbrew->value("SP_TIME").toInt());
    ui->sbSparge3->setValue(pEbrew->RegEbrew->value("SP_MPY").toDouble());

    //-------------------------------
    // Boiling
    //-------------------------------
    ui->sbBoil1->setValue(pEbrew->RegEbrew->value("BOIL_MIN_TEMP").toInt());
    ui->sbBoil2->setValue(pEbrew->RegEbrew->value("SP_PREBOIL").toDouble());
    ui->sbBoil3->setValue(pEbrew->RegEbrew->value("BOIL_DETECT").toDouble());
    ui->sbBoil4->setValue(pEbrew->RegEbrew->value("SP_BOIL").toDouble());
    ui->sbBoil5->setValue(pEbrew->RegEbrew->value("LIMIT_BOIL").toInt());
    ui->cbBoil1->setChecked(pEbrew->RegEbrew->value("CB_Boil_Rest").toInt() ? true : false);
    ui->cbHopAlarm->setChecked(pEbrew->RegEbrew->value("CB_Hop_Alarm").toInt() ? true : false);

    //-------------------------------
    // Clean in Place (CIP)
    //-------------------------------
    ui->sbCip1->setValue(pEbrew->RegEbrew->value("CIP_SP").toInt());
    ui->sbCip2->setValue(pEbrew->RegEbrew->value("CIP_CIRC_TIME").toInt());
    ui->sbCip3->setValue(pEbrew->RegEbrew->value("CIP_REST_TIME").toInt());
    ui->sbCip4->setValue(pEbrew->RegEbrew->value("CIP_OUT_TIME").toInt());
    ui->sbCip5->setValue(pEbrew->RegEbrew->value("CIP_INP_TIME").toInt());
} // Constructor

DialogBrewDaySettings::~DialogBrewDaySettings()
{
    delete ui;
} // DialogBrewDaySettings::~DialogBrewDaySettings()

void DialogBrewDaySettings::on_buttonBox_accepted()
{
    //-------------------------------
    // Mashing
    //-------------------------------
    pEbrew->RegEbrew->setValue("TOffset0"     ,ui->sbMash1->value());
    pEbrew->RegEbrew->setValue("TOffset"      ,ui->sbMash2->value());
    pEbrew->RegEbrew->setValue("TOffset2"     ,ui->sbMash3->value());
    pEbrew->RegEbrew->setValue("HLT_Bcap"     ,ui->sbMash4->value());
    pEbrew->RegEbrew->setValue("PREHEAT_TIME" ,ui->sbMash5->value());
    pEbrew->RegEbrew->setValue("CB_dpht"      ,ui->cbMash1->isChecked() ? 1 : 0);
    pEbrew->RegEbrew->setValue("CB_Malt_First",ui->cbMash2->isChecked() ? 1 : 0);
    pEbrew->RegEbrew->setValue("CB_Mash_Rest" ,ui->cbMash3->isChecked() ? 1 : 0);
    pEbrew->RegEbrew->setValue("CB_pumps_on"  ,ui->cbMash4->isChecked() ? 1 : 0);

    //-------------------------------
    // Sparging
    //-------------------------------
    pEbrew->RegEbrew->setValue("SP_BATCHES",ui->sbSparge1->value());
    pEbrew->RegEbrew->setValue("SP_TIME"   ,ui->sbSparge2->value());
    pEbrew->RegEbrew->setValue("SP_MPY"    ,ui->sbSparge3->value());

    //-------------------------------
    // Boiling
    //-------------------------------
    pEbrew->RegEbrew->setValue("BOIL_MIN_TEMP",ui->sbBoil1->value());
    pEbrew->RegEbrew->setValue("SP_PREBOIL"   ,ui->sbBoil2->value());
    pEbrew->RegEbrew->setValue("BOIL_DETECT"  ,ui->sbBoil3->value());
    pEbrew->RegEbrew->setValue("SP_BOIL"      ,ui->sbBoil4->value());
    pEbrew->RegEbrew->setValue("LIMIT_BOIL"   ,ui->sbBoil5->value());
    pEbrew->RegEbrew->setValue("CB_Boil_Rest" ,ui->cbBoil1->isChecked() ? 1 : 0);
    pEbrew->RegEbrew->setValue("CB_Hop_Alarm" ,ui->cbHopAlarm->isChecked() ? 1 : 0);

    //-------------------------------
    // Clean in Place (CIP)
    //-------------------------------
    pEbrew->RegEbrew->setValue("CIP_SP"       ,ui->sbCip1->value());
    pEbrew->RegEbrew->setValue("CIP_CIRC_TIME",ui->sbCip2->value());
    pEbrew->RegEbrew->setValue("CIP_REST_TIME",ui->sbCip3->value());
    pEbrew->RegEbrew->setValue("CIP_OUT_TIME" ,ui->sbCip4->value());
    pEbrew->RegEbrew->setValue("CIP_INP_TIME" ,ui->sbCip5->value());

    pEbrew->initBrewDaySettings(); // update (sparge) timers
} // DialogBrewDaySettings::on_buttonBox_accepted()

void DialogBrewDaySettings::on_cbMash1_stateChanged(int arg1)
{   // Dynamic Preheat Timing checkbox
    if (arg1)
    {
        ui->lblMash4->setEnabled(true);
        ui->sbMash4->setEnabled(true);
        ui->lblMash5->setEnabled(false);
        ui->sbMash5->setEnabled(false);
    } // if
    else
    {
        ui->lblMash4->setEnabled(false);
        ui->sbMash4->setEnabled(false);
        ui->lblMash5->setEnabled(true);
        ui->sbMash5->setEnabled(true);
    } // else
} // DialogBrewDaySettings::on_cbMash1_stateChanged()
