#include "dialogoptionssystemsettings.h"
#include "ui_dialogoptionssystemsettings.h"
#include <QString>

DialogOptionsSystemSettings::DialogOptionsSystemSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptionsSystemSettings)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);

    //-------------------------
    // Heater Mode
    //-------------------------
    switch (pEbrew->RegEbrew->value("SYSTEM_MODE").toInt())
    {
        case GAS_MODULATING    : ui->rb1->setChecked(true); set_mode1(); break;
        case GAS_NON_MODULATING: ui->rb2->setChecked(true); set_mode2(); break;
        case ELECTRICAL_HEATING: ui->rb3->setChecked(true); set_mode3(); break;
    } // switch
    ui->sb11->setValue(pEbrew->RegEbrew->value("GAS_MOD_PWM_LLIMIT").toInt());
    ui->sb12->setValue(pEbrew->RegEbrew->value("GAS_MOD_PWM_HLIMIT").toInt());
    ui->sb21->setValue(pEbrew->RegEbrew->value("GAS_NON_MOD_LLIMIT").toInt());
    ui->sb22->setValue(pEbrew->RegEbrew->value("GAS_NON_MOD_HLIMIT").toInt());
    ui->sb31->setValue(pEbrew->RegEbrew->value("TTRIAC_LLIM").toInt());
    ui->sb32->setValue(pEbrew->RegEbrew->value("TTRIAC_HLIM").toInt());
    //-------------------------
    // Communications
    //-------------------------
    ui->cbCommCh->setCurrentIndex(pEbrew->RegEbrew->value("COMM_CHANNEL").toInt());
    ui->leComPort->setText(pEbrew->RegEbrew->value("COM_PORT_SETTINGS").toString());
    ui->leIPaddr->setText(pEbrew->RegEbrew->value("UDP_IP_PORT").toString());
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
    ui->cbDbgLog->setChecked(pEbrew->RegEbrew->value("CB_DEBUG_COM_PORT").toInt() == 1);
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
    else pEbrew->RegEbrew->setValue("SYSTEM_MODE",ELECTRICAL_HEATING);
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",ui->sb11->value());
    pEbrew->RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",ui->sb12->value());
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_LLIMIT",ui->sb21->value());
    pEbrew->RegEbrew->setValue("GAS_NON_MOD_HLIMIT",ui->sb22->value());
    pEbrew->RegEbrew->setValue("TTRIAC_LLIM",ui->sb31->value());
    pEbrew->RegEbrew->setValue("TTRIAC_HLIM",ui->sb32->value());
    //-------------------------
    // Communications
    //-------------------------
    pEbrew->RegEbrew->setValue("COMM_CHANNEL",ui->cbCommCh->currentIndex());
    pEbrew->RegEbrew->setValue("COM_PORT_SETTINGS",ui->leComPort->text());
    pEbrew->RegEbrew->setValue("UDP_IP_PORT",ui->leIPaddr->text());
    pEbrew->RegEbrew->setValue("CB_DEBUG_COM_PORT",ui->cbDbgLog->isChecked() ? 1 : 0);
    //-------------------------
    // Brew-kettle Sizes
    //-------------------------
    pEbrew->RegEbrew->setValue("VHLT_MAX",ui->sbHltVol->value());
    pEbrew->RegEbrew->setValue("VMLT_MAX",ui->sbMltVol->value());
    pEbrew->RegEbrew->setValue("VBOIL_MAX",ui->sbBkVol->value());
    pEbrew->setKettleNames();  // Init. titles of kettles with volumes found in Registry
} // DialogOptionsSystemSettings::on_buttonBox_accepted()

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
