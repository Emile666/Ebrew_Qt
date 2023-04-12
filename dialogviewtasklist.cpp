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
    QByteArray s1, s2, s3;
    pEbrew->schedulerEbrew->stop();  // stop scheduler for a moment
    pEbrew->sleep(40);               // give scheduler time to stop all communications
    pEbrew->commPortWrite("S2");     // S2 = list all tasks from Ebrew hardware

    pEbrew->ReadDataAvailable = false;
    ui->teTaskList->setText(pEbrew->schedulerEbrew->list_all_tasks()); // PC task-list

    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_SX_READ_RETRIES))
    {
        pEbrew->sleep(TIMEOUT_SX_READS);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        s2 = pEbrew->ReadData;
        pEbrew->ReadDataAvailable = false;
        QByteArrayList list = s2.split(','); // split array in sub-arrays
        s2.clear();
        for (int i = 0; i < list.size(); i++)
        {
            QByteArray x = list.at(i);
            if ((i%5) == 0)
            {   // task name
                x.insert(0,13-x.size(),' ');
                s2.append(x);
            } // if
            else if ((i%5) == 1)
            {   // period-time
                x.insert(0,7-x.size(),' ');
                s2.append(x);
            } // else if
            else if ((i%5) == 2)
            {   // status
                x.prepend("0x0");
                x.insert(0,5-x.size(),' ');
                s2.append(x);
            } // else if
            else if ((i%5) == 3)
            {   // actual time
                x.insert(0,6-x.size(),' ');
                s2.append(x);
            } // else if
            else
            {   // max. time
                x.insert(0,6-x.size(),' ');
                s2.append(x);
                s2.append('\n');
            } // else
        } // for i
        s2.prepend("-------------------------------------\n");
        s2.prepend("Task-Name      T(ms) Stat T(ms) M(ms)\n");
        ui->teTaskList2->setText(s2);
    }
    else ui->teTaskList2->setText("S2 read-error");

    count = 0;
    pEbrew->commPortWrite("S1");     // S1 = list all I2C-devices from Ebrew hardware
    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_SX_READ_RETRIES))
    {
        pEbrew->sleep(TIMEOUT_SX_READS);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        pEbrew->ReadDataAvailable = false;
        s1 = pEbrew->ReadData;
        //qDebug() << "s1 = " << s1;
        int j;
        while ((j = s1.indexOf(" I2C")) > -1)
        {
            s1.remove(j,1);    // remove space
            s1.insert(j,'\n'); // insert newline
        } // while
        s1.append("\n\n");
        s1.append("0x30,0x32,0x34,0x36: DS2482   I2C to 1-Wire Master\n");
        s1.append("0x90,0x92,0x94,0x96: LM92     I2C temperature sensor\n\n");
        ui->teTaskList3->setText(s1);
    } // if
    else ui->teTaskList3->setText("S1 read-error");

    count = 0;
    pEbrew->commPortWrite("S3");     // S3 = list all One-Wire devices from Ebrew hardware
    s3        = "OW1: ";
    int owcnt = 2;
    while (pEbrew->comPortIsOpen && !pEbrew->ReadDataAvailable && (count++ < MAX_SX_READ_RETRIES))
    {
        pEbrew->sleep(TIMEOUT_SX_READS);
    } // while
    if (pEbrew->ReadDataAvailable)
    {
        pEbrew->ReadDataAvailable = false;
        s3.append(pEbrew->ReadData);
        int j;
        while ((j = s3.indexOf('.')) > -1)
        {
            s3.remove(j,1);    // remove '.'
            if (j == s3.size())
                 s3.insert(j,'\n');
            else s3.insert(j,QString("\nOW%1: ").arg(owcnt++)); // insert newline
        } // while
        ui->teTaskList3->setText(s1 + s3);
    } // if
    else ui->teTaskList3->setText("S3 read-error");

    pEbrew->schedulerEbrew->start(); // restart scheduler
} // DialogViewTaskList::readTaskListData()

void DialogViewTaskList::on_refreshButton_clicked()
{   // Refresh button pressed
    readTaskListData(); // read task-lists and display in text-edit objects
} // DialogViewTaskList::on_refreshButton_clicked()
