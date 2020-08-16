/**************************************************************************************
** Filename    : dialogoptionssystemsettings.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen
**               with which to change system-wide parameters, such as communications.
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
#include "dialogoptionssystemsettings.h"
#include "ui_dialogoptionssystemsettings.h"
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

DialogOptionsSystemSettings::DialogOptionsSystemSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsSystemSettings)
{
    QString string;
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);

    //-------------------------
    // Heater Mode
    //-------------------------
    switch (systemMode = pEbrew->RegEbrew->value("SYSTEM_MODE").toInt())
    {
        case GAS_MODULATING    : ui->rb1->setChecked(true); set_mode1(); break;
        case GAS_NON_MODULATING: ui->rb2->setChecked(true); set_mode2(); break;
        case ELECTRICAL_HEATING: ui->rb3->setChecked(true); set_mode3(); break;
    } // switch
    ui->sb11->setValue(gasModPwmLlimit = pEbrew->RegEbrew->value("GAS_MOD_PWM_LLIMIT").toInt());
    ui->sb12->setValue(gasModPwmHlimit = pEbrew->RegEbrew->value("GAS_MOD_PWM_HLIMIT").toInt());
    ui->sb21->setValue(gasNonModLlimit = pEbrew->RegEbrew->value("GAS_NON_MOD_LLIMIT").toInt());
    ui->sb22->setValue(gasNonModHlimit = pEbrew->RegEbrew->value("GAS_NON_MOD_HLIMIT").toInt());
    ui->sb31->setValue(triacLlimit = pEbrew->RegEbrew->value("TTRIAC_LLIM").toInt());
    ui->sb32->setValue(triacHlimit = pEbrew->RegEbrew->value("TTRIAC_HLIM").toInt());
    //-------------------------
    // Communications
    //-------------------------
    const auto infos = QSerialPortInfo::availablePorts();
    string = "Available COM Ports:\n";
    for (const QSerialPortInfo &info : infos)
    {
        string.append(info.portName() + ": " + info.description() + "\n");
    } // for
    ui->lblAvailable->setText(string);
    ui->cbCommCh->setCurrentIndex(commChannel = pEbrew->RegEbrew->value("COMM_CHANNEL").toInt());
    ui->leComPort->setText(commSettings = pEbrew->RegEbrew->value("COM_PORT_SETTINGS").toString());
    ui->leIPaddr->setText(commUdp = pEbrew->RegEbrew->value("UDP_IP_PORT").toString());
    if (ui->cbCommCh->currentIndex() == 0)
    {   // Ethernet (UDP)
        ui->lblComPort->setEnabled(false);
        ui->leComPort->setEnabled(false);
        ui->lblIPaddr->setEnabled(true);
        ui->leIPaddr->setEnabled(true);
    } // if
    else
    {   // COMx (Virtual USB)
        ui->lblComPort->setEnabled(true);
        ui->leComPort->setEnabled(true);
        ui->lblIPaddr->setEnabled(false);
        ui->leIPaddr->setEnabled(false);
    } // else
    commCb = pEbrew->RegEbrew->value("CB_DEBUG_COM_PORT").toInt();
    ui->cbDbgLog->setChecked(commCb == 1);

    //-------------------------
    // Brew-kettle Sizes
    //-------------------------
    ui->sbHltVol->setValue(pEbrew->RegEbrew->value("VHLT_MAX").toInt());
    ui->sbMltVol->setValue(pEbrew->RegEbrew->value("VMLT_MAX").toInt());
    ui->sbBkVol->setValue(pEbrew->RegEbrew->value("VBOIL_MAX").toInt());
} // Constructor

DialogOptionsSystemSettings::~DialogOptionsSystemSettings()
{
    delete ui;
}

void DialogOptionsSystemSettings::on_buttonBox_accepted()
{
    //-------------------------
    // Heater Mode
    //-------------------------
    if (ui->rb1->isChecked())
         pEbrew->RegEbrew->setValue("SYSTEM_MODE",GAS_MODULATING);
    else if (ui->rb2->isChecked())
         pEbrew->RegEbrew->setValue("SYSTEM_MODE",GAS_NON_MODULATING);
    else pEbrew->RegEbrew->setValue("SYSTEM_MODE",ELECTRICAL_HEATING);  // Parameter 0
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_LLIMIT",ui->sb21->value()); // Parameter 1
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_HLIMIT",ui->sb22->value()); // Parameter 2
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",ui->sb11->value()); // Parameter 3
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",ui->sb12->value()); // Parameter 4
    pEbrew->RegEbrew->setValue("TTRIAC_LLIM",ui->sb31->value());        // Parameter 5
    pEbrew->RegEbrew->setValue("TTRIAC_HLIM",ui->sb32->value());        // Parameter 6
    pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
    pEbrew->sleep(10);               // give scheduler time to stop all communications
    sendChangedValueToEbrewHW(pEbrew->RegEbrew->value("SYSTEM_MODE").toUInt()       ,systemMode     ,0);
    sendChangedValueToEbrewHW(pEbrew->RegEbrew->value("GAS_NON_MOD_LLIMIT").toUInt(),gasNonModLlimit,1);
    sendChangedValueToEbrewHW(pEbrew->RegEbrew->value("GAS_NON_MOD_HLIMIT").toUInt(),gasNonModHlimit,2);
    sendChangedValueToEbrewHW(pEbrew->RegEbrew->value("GAS_MOD_PWM_LLIMIT").toUInt(),gasModPwmLlimit,3);
    sendChangedValueToEbrewHW(pEbrew->RegEbrew->value("GAS_MOD_PWM_HLIMIT").toUInt(),gasModPwmHlimit,4);
    sendChangedValueToEbrewHW(100*pEbrew->RegEbrew->value("TTRIAC_LLIM").toUInt()       ,100*triacLlimit    ,5); // in E-2 °C
    sendChangedValueToEbrewHW(100*pEbrew->RegEbrew->value("TTRIAC_HLIM").toUInt()       ,100*triacHlimit    ,6); // in E-2 °C

    //-------------------------
    // Communications
    //-------------------------
    pEbrew->RegEbrew->setValue("COMM_CHANNEL",ui->cbCommCh->currentIndex());
    pEbrew->RegEbrew->setValue("COM_PORT_SETTINGS",ui->leComPort->text());
    pEbrew->RegEbrew->setValue("UDP_IP_PORT",ui->leIPaddr->text());
    int x = ui->cbDbgLog->isChecked() ? 1 : 0;
    pEbrew->RegEbrew->setValue("CB_DEBUG_COM_PORT",x);
    if ((ui->cbCommCh->currentIndex() != commChannel) || (ui->leComPort->text() != commSettings) || (ui->leIPaddr->text() != commUdp) || (x != commCb))
    {   // Communication settings have changed
        pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
        pEbrew->sleep(10);               // give scheduler time to stop all communications
        pEbrew->commPortClose();         // this also closes the com-port log-file
        pEbrew->commPortOpen();          // open COM port with new settings
    } // if
    pEbrew->schedulerEbrew->start(); // restart scheduler

    //-------------------------
    // Brew-kettle Sizes
    //-------------------------
    pEbrew->RegEbrew->setValue("VHLT_MAX",ui->sbHltVol->value());
    pEbrew->RegEbrew->setValue("VMLT_MAX",ui->sbMltVol->value());
    pEbrew->RegEbrew->setValue("VBOIL_MAX",ui->sbBkVol->value());
    pEbrew->setKettleNames();  // Init. titles of kettles with volumes found in Registry
} // DialogOptionsSystemSettings::on_buttonBox_accepted()

void DialogOptionsSystemSettings::sendChangedValueToEbrewHW(uint16_t val1, uint16_t val2, uint8_t nr)
{
    if (val1 != val2)
    {
        QString string = QString("N%1 %2").arg(nr).arg(val1); // send new value to Ebrew HW
        pEbrew->commPortWrite(string.toUtf8());
    } // if
} // DialogOptionsSystemSettings::sendChangedValueToEbrewHW()

void DialogOptionsSystemSettings::set_mode0(void)
{
    ui->lbl11->setEnabled(false);
    ui->lbl12->setEnabled(false);
    ui->sb11->setEnabled(false);
    ui->sb12->setEnabled(false);
    ui->lbl21->setEnabled(false);
    ui->lbl22->setEnabled(false);
    ui->sb21->setEnabled(false);
    ui->sb22->setEnabled(false);
    ui->lbl31->setEnabled(false);
    ui->lbl32->setEnabled(false);
    ui->sb31->setEnabled(false);
    ui->sb32->setEnabled(false);
} // DialogOptionsSystemSettings::set_mode0()

void DialogOptionsSystemSettings::set_mode1(void)
{
    set_mode0();
    ui->lbl11->setEnabled(true);
    ui->lbl12->setEnabled(true);
    ui->sb11->setEnabled(true);
    ui->sb12->setEnabled(true);
} // DialogOptionsSystemSettings::set_mode1()

void DialogOptionsSystemSettings::set_mode2(void)
{
    set_mode0();
    ui->lbl21->setEnabled(true);
    ui->lbl22->setEnabled(true);
    ui->sb21->setEnabled(true);
    ui->sb22->setEnabled(true);
} // DialogOptionsSystemSettings::set_mode2()

void DialogOptionsSystemSettings::set_mode3(void)
{
    set_mode0();
    ui->lbl31->setEnabled(true);
    ui->lbl32->setEnabled(true);
    ui->sb31->setEnabled(true);
    ui->sb32->setEnabled(true);
} // DialogOptionsSystemSettings::set_mode3()

void DialogOptionsSystemSettings::on_rb1_clicked()
{
    set_mode1();
} // DialogOptionsSystemSettings::on_rb1_clicked()

void DialogOptionsSystemSettings::on_rb2_clicked()
{
    set_mode2();
} // DialogOptionsSystemSettings::on_rb2_clicked()


void DialogOptionsSystemSettings::on_rb3_clicked()
{
    set_mode3();
} // DialogOptionsSystemSettings::on_rb3_clicked()

void DialogOptionsSystemSettings::on_cbCommCh_currentIndexChanged(int index)
{
    if (index > 0)
    {
        ui->lblComPort->setEnabled(true);
        ui->leComPort->setEnabled(true);
        ui->lblIPaddr->setEnabled(false);
        ui->leIPaddr->setEnabled(false);
    } // if
    else
    {
        ui->lblComPort->setEnabled(false);
        ui->leComPort->setEnabled(false);
        ui->lblIPaddr->setEnabled(true);
        ui->leIPaddr->setEnabled(true);
    } // else
} // DialogOptionsSystemSettings::on_cbCommCh_currentIndexChanged()
