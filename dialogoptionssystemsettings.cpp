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
    uint8_t htrh = pEbrew->RegEbrew->value("HEATERSH").toInt();
    uint8_t htrb = pEbrew->RegEbrew->value("HEATERSB").toInt();
    ui->cb11->setChecked(htrh & 0x01);
    ui->cb12->setChecked(htrh & 0x02);
    ui->cb13->setChecked(htrh & 0x04);
    ui->cb14->setChecked(htrh & 0x08);
    ui->cb15->setChecked(htrh & 0x10);

    ui->cb21->setChecked(htrb & 0x01);
    ui->cb22->setChecked(htrb & 0x02);
    ui->cb23->setChecked(htrb & 0x04);
    ui->cb24->setChecked(htrb & 0x08);
    ui->cb25->setChecked(htrb & 0x10);

    ui->sb11->setValue(pEbrew->RegEbrew->value("GAS_MOD_PWM_LLIMIT").toInt());
    ui->sb12->setValue(pEbrew->RegEbrew->value("GAS_MOD_PWM_HLIMIT").toInt());
    ui->sb21->setValue(pEbrew->RegEbrew->value("GAS_NON_MOD_LLIMIT").toInt());
    ui->sb22->setValue(pEbrew->RegEbrew->value("GAS_NON_MOD_HLIMIT").toInt());
    ui->sb31->setValue(pEbrew->RegEbrew->value("TTRIAC_LLIM").toInt());
    ui->sb32->setValue(pEbrew->RegEbrew->value("TTRIAC_HLIM").toInt());
    //-------------------------
    // Communications
    //-------------------------
    const auto infos = QSerialPortInfo::availablePorts();
    string = "Available COM Ports:\n";
    for (const QSerialPortInfo &info : infos)
    {
        string.append(info.portName() + ": " + info.description() + "\n");
    } // for
    if (pEbrew->ebrewHwIp != QHostAddress::AnyIPv4)
    {
        string.append("\nIP-address:");
        string.append(pEbrew->ebrewHwIp.toString());
    } // if
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
    ui->sbHltMin->setValue(pEbrew->RegEbrew->value("VHLT_MIN").toInt());
    ui->sbBkMin->setValue(pEbrew->RegEbrew->value("VBOIL_MIN").toInt());
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
    uint8_t htrh = ui->cb11->isChecked() + (ui->cb12->isChecked() << 1) + (ui->cb13->isChecked() << 2) +
                   (ui->cb14->isChecked() << 3) + (ui->cb15->isChecked() << 4);
    uint8_t htrb = ui->cb21->isChecked() + (ui->cb22->isChecked() << 1) + (ui->cb23->isChecked() << 2) +
                   (ui->cb24->isChecked() << 3) + (ui->cb25->isChecked() << 4);
    pEbrew->RegEbrew->setValue("HEATERSH",htrh);
    pEbrew->hlt->setHeatingOptions(htrh);
    pEbrew->RegEbrew->setValue("HEATERSB",htrb);
    pEbrew->boil->setHeatingOptions(htrb);
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_LLIMIT",ui->sb21->value()); // Parameter 1
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_HLIMIT",ui->sb22->value()); // Parameter 2
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",ui->sb11->value()); // Parameter 3
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",ui->sb12->value()); // Parameter 4
    pEbrew->RegEbrew->setValue("TTRIAC_LLIM",ui->sb31->value());        // Parameter 5
    pEbrew->RegEbrew->setValue("TTRIAC_HLIM",ui->sb32->value());        // Parameter 6
    pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
    pEbrew->sleep(10);               // give scheduler time to stop all communications

    //-------------------------
    // Communications
    //-------------------------
    pEbrew->RegEbrew->setValue("COMM_CHANNEL",ui->cbCommCh->currentIndex());
    pEbrew->RegEbrew->setValue("COM_PORT_SETTINGS",ui->leComPort->text());
    pEbrew->RegEbrew->setValue("UDP_IP_PORT",ui->leIPaddr->text());
    pEbrew->splitIpAddressPort(); // Split Registry IP-address and port-number

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
    // Brew-kettle Volumes
    //-------------------------
    pEbrew->RegEbrew->setValue("VHLT_MAX",ui->sbHltVol->value());
    pEbrew->RegEbrew->setValue("VMLT_MAX",ui->sbMltVol->value());
    pEbrew->RegEbrew->setValue("VBOIL_MAX",ui->sbBkVol->value());
    pEbrew->RegEbrew->setValue("VHLT_MIN",ui->sbHltMin->value());
    pEbrew->RegEbrew->setValue("VBOIL_MIN",ui->sbBkMin->value());
    pEbrew->setKettleVolumes();  // Init. titles of kettles with volumes found in Registry
} // DialogOptionsSystemSettings::on_buttonBox_accepted()

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
