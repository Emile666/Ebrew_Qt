/**************************************************************************************
** Filename    : dialogeditterminal.cpp
** Author      : Emile
** Purpose     : This file contains a terminal editor with which to send commands
**               directly to the Ebrew hardware.
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
#include "dialogeditterminal.h"
#include "ui_dialogeditterminal.h"
#include <QKeyEvent>

DialogEditTerminal::DialogEditTerminal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditTerminal)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);
    ui->setupUi(this);
    ui->teTerminal->clear();
    ui->teTerminal->setFocus();
    ui->teTerminal->installEventFilter(this); // install EventFilter to catch QTextEdit key-presses
    cmdString.clear();
    pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
    pEbrew->sleep(40);               // give scheduler time to stop all communications
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    ui->teTerminal->setPalette(p);

    // Disconnect the serialport readyRead slot from Ebrew and connect it to this dialog screen
    disconnect(pEbrew->serialPort, &QSerialPort::readyRead,pEbrew,&MainEbrew::commPortRead);
    connect(   pEbrew->serialPort, &QSerialPort::readyRead,this  ,&DialogEditTerminal::readSerialPort);
    // Disconnect the UdpSocket readyRead slot from Ebrew and connect it to this dialog screen
    disconnect(pEbrew->udpSocket , &QUdpSocket::readyRead ,pEbrew,&MainEbrew::commPortRead);
    connect(   pEbrew->udpSocket , &QUdpSocket::readyRead ,this  ,&DialogEditTerminal::readSerialPort);
} // DialogEditTerminal::DialogEditTerminal()

DialogEditTerminal::~DialogEditTerminal()
{
    delete ui;
} // DialogEditTerminal::~DialogEditTerminal()

bool DialogEditTerminal::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) )
        {
            //Enter or return was pressed
            pEbrew->commPortWrite(cmdString); // write to comm. channel
            cmdString.clear();                // clear command string
            return QObject::eventFilter(obj, event);
        } // if
        else if ((key->key()==Qt::Key_Backspace) || (key->key()==Qt::Key_Left) ||
                 (key->key()==Qt::Key_Right)     || (key->key()==Qt::Key_Up)   ||
                 (key->key()==Qt::Key_Down))
        { // do nothing
        }
        else
        {   // a normal character
            cmdString.append(key->text()); // add char. to command string
            return QObject::eventFilter(obj, event);
        } // else
        return true;
    } // if
    else
    {   // not a keypress event, let Qt handle it
        return QObject::eventFilter(obj, event);
    } // else
    return false;
} // DialogEditTerminal::eventFilter()

void DialogEditTerminal::readSerialPort(void)
{
    QByteArray ba;
    ui->teTerminal->setTextColor(Qt::yellow);
    if (pEbrew->RegEbrew->value("COMM_CHANNEL").toInt() > 0)
    {   // any of the virtual USB com ports
        ba = pEbrew->serialPort->readAll();
    } // if
    else
    {   // Ethernet UDP port
        ba.resize(pEbrew->udpSocket->pendingDatagramSize());
        pEbrew->udpSocket->readDatagram(ba.data(), ba.size(),&(pEbrew->ebrewHwIp));
    } // else
    ui->teTerminal->append(ba);
    ui->teTerminal->setTextColor(Qt::green);
} // DialogEditTerminal::readSerialPort()

void DialogEditTerminal::on_buttonBox_clicked(QAbstractButton *)
{
    connect(pEbrew->udpSocket , &QUdpSocket::readyRead ,pEbrew,&MainEbrew::commPortRead); // Restore Ebrew udp-socket connection
    connect(pEbrew->serialPort, &QSerialPort::readyRead,pEbrew,&MainEbrew::commPortRead); // Restore Ebrew serialport connection
    pEbrew->commPortClose();
    pEbrew->commPortOpen();
    pEbrew->schedulerEbrew->start(); // restart Ebrew scheduler
} // DialogEditTerminal::on_buttonBox_clicked()
