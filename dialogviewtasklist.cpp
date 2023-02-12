/**************************************************************************************
** Filename    : dialogviewtasklist.cpp
** Author      : Emile
** Purpose     : This file contains a screen that shows
**               scheduler tasks, both from the PC-program and from the EBrew hardware.
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
#include "dialogviewtasklist.h"
#include "ui_dialogviewtasklist.h"
#include <QDebug>

DialogViewTaskList::DialogViewTaskList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogViewTaskList)
{
    ui->setupUi(this);
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    readTaskListData(); // read task-lists and display in text-edit objects
} // DialogViewTaskList::DialogViewTaskList()

DialogViewTaskList::~DialogViewTaskList()
{
    delete ui;
} // DialogViewTaskList::~DialogViewTaskList()

void DialogViewTaskList::readTaskListData(void)
{
    int count = 0;
    QByteArray s3, s2, s4;
    pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
    pEbrew->sleep(40);               // give scheduler time to stop all communications
    pEbrew->commPortWrite("S3");     // S3 = list all tasks from Ebrew hardware

    ui->teTaskList->setText(pEbrew->schedulerEbrew->list_all_tasks()); // PC task-list

    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_READ_RETRIES))
    {
        pEbrew->sleep(LONG_READ_TIMEOUT);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        s3 = pEbrew->ReadData;
        QByteArrayList list = s3.split(','); // split array in sub-arrays
        s3.clear();
        for (int i = 0; i < list.size(); i++)
        {
            QByteArray x = list.at(i);
            if (i == 0)
            {   // init. s3 bytearray
                s3 = x;
                s3.insert(0,13-x.size(),' ');
            } // if
            else if (((i % 4) == 0) && (i < list.size()-1))
            {   // name with max-time prepended
                int j = 0;
                while ((j < x.size()) && QChar(x[j]).isDigit()) j++;
                if (j < x.size())
                {
                    x.insert(j,14-x.size(),' ');
                    x.insert(j,'\n');
                    x.insert(j-1,6-j,' ');
                } // if
                s3.append(x);
            } // else if
            else if ((i%4) == 1)
            {   // period-time
                x.insert(0,7-x.size(),' ');
                s3.append(x);
            } // else if
            else if ((i%4) == 2)
            {   // status
                x.prepend("0x0");
                x.insert(0,5-x.size(),' ');
                s3.append(x);
            } // else if
            else
            {   // actual time and max-time
                x.insert(0,6-x.size(),' ');
                s3.append(x);
            } // else if
        } // for i
        s3.prepend("-------------------------------------\n");
        s3.prepend("Task-Name      T(ms) Stat T(ms) M(ms)\n");

        ui->teTaskList2->setText(s3);
    }
    else ui->teTaskList2->setText("S3 read-error");

    pEbrew->commPortWrite("S2");     // S2 = list all I2C-devices from Ebrew hardware
    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_READ_RETRIES))
    {
        pEbrew->sleep(LONG_READ_TIMEOUT);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        s2 = pEbrew->ReadData;
        //qDebug() << "s2 = " << s2;
        int j;
        while ((j = s2.indexOf(" I2C")) > -1)
        {
            s2.remove(j,1);    // remove space
            s2.insert(j,'\n'); // insert newline
        } // while
        s2.append("\n\n");
        s2.append("0x30,0x32,0x34,0x36: DS2482   I2C to 1-Wire Master\n");
        s2.append("0x40               : MCP23017 I2C 16-bit GPIO\n");
        s2.append("0x90,0x92,0x94,0x96: LM92     I2C temperature sensor\n");
        s2.append("0xe0               : PCA9544  I2C bus multiplexer 4 ch.\n");
        ui->teTaskList3->setText(s2);
    } // if
    else ui->teTaskList3->setText("S2 read-error");

    pEbrew->commPortWrite("S4");     // S2 = list all One-Wire devices from Ebrew hardware
    count = 0;
    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_READ_RETRIES))
    {
        pEbrew->sleep(LONG_READ_TIMEOUT);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        s4 = pEbrew->ReadData;
        ui->teTaskList3->setText(s2 + "\n\n" + s4);
    } // if
    else ui->teTaskList3->setText("S4 read-error");

    pEbrew->schedulerEbrew->start(); // restart scheduler
} // DialogViewTaskList::readTaskListData()

void DialogViewTaskList::on_refreshButton_clicked()
{   // Refresh button pressed
    readTaskListData(); // read task-lists and display in text-edit objects
} // DialogViewTaskList::on_refreshButton_clicked()
