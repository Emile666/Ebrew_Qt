/**************************************************************************************
** Filename    : dialogviewprogress.cpp
** Author      : Emile
** Purpose     : This file contains an information screen
**               for tracking status of mashing, sparging and boiling.
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
#include <QTimer>
#include "dialogviewprogress.h"
#include "ui_dialogviewprogress.h"

DialogViewProgress::DialogViewProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogViewProgress)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);
    ui->setupUi(this);
    onUpdateProgress(); // start directly 1st time
    timerId = startTimer(5000); // update every 5 seconds
}

DialogViewProgress::~DialogViewProgress()
{
    killTimer(timerId); // delete the timer
    delete ui;
}

void DialogViewProgress::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
        onUpdateProgress();
} // DialogViewProgress::timerEvent()

void DialogViewProgress::onUpdateProgress(void)
{
    mash_schedule *p; // pointer naar maisch_schedule structure
    QString string;
    QFont   font("Courier New",8);

    ui->teMash->clear();
    bool mashActive = (pEbrew->ms_idx < pEbrew->ms_tot - 1) || (pEbrew->ms[pEbrew->ms_tot-1].timer < pEbrew->ms[pEbrew->ms_tot-1].time);
    if (mashActive)
    {
        ui->teMash->setEnabled(true);
        ui->lblMash->setEnabled(true);
    }
    else
    {   // mashing not active anymore
        ui->teMash->setEnabled(false);
        ui->lblMash->setEnabled(false);
    } // else

    font.setBold(false);
    ui->teMash->setCurrentFont(font);
    ui->teMash->setTextColor(Qt::black);
    ui->teMash->setText(MashTitle);
    //---------------------------------------------------------------------
    // Sample time of ms[] update is 1 second.
    // Time of ms[i].time was converted to seconds in readMashSchemeFile().
    //---------------------------------------------------------------------
    for (int i = 0; i < pEbrew->ms_tot; i++)
    {
        p = &pEbrew->ms[i];
        string = QString("%1 %2 %3 %4 %5  ").arg(i,2)
                                            .arg(p->temp ,5,'f',0)
                                            .arg(p->time ,5,'f',0)
                                            .arg(p->preht,5)
                                            .arg(p->timer == NOT_STARTED ? 0 : p->timer,5);
        if (p->timer == NOT_STARTED)
        {
            string.append("Not Started ");
        } // if
        else if (p->timer < p->time)
        {
            string.append("Running     ");
        } // else
        else
        {
            string.append("Time-Out    ");
        } // else
        string.append(p->time_stamp);
        if (mashActive && (i == pEbrew->ms_idx))
        {
            ui->teMash->setTextColor(Qt::blue);
            font.setBold(true);
            string = "->" + string;
        } // if
        else
        {
            ui->teMash->setTextColor(Qt::black);
            font.setBold(false);
            string = "  " + string;
        } // else
        ui->teMash->setCurrentFont(font);
        ui->teMash->append(string);
    } // for i

    // SPARGE PROGRESS
    bool spargeActive = (pEbrew->ebrew_std >= S05_SPARGE_TIMER_RUNNING) && (pEbrew->ebrew_std < S09_EMPTY_MLT);
    // TEST
//    spargeActive = true;
//    pEbrew->mlt2boil << "14:59:59" << "15:51:32" << "15:52:58"<< "15:54:24";
//    pEbrew->hlt2mlt  << "15:50:24" << "15:51:52" << "15:53:16"<< "15:54:46";
    // TEST
    if (spargeActive)
    {
        ui->teSparge->setEnabled(true);
        ui->lblSparge->setEnabled(true);
    } // if
    else
    {
        ui->teSparge->setEnabled(false);
        ui->lblSparge->setEnabled(false);
    } // else
    font.setBold(false);
    ui->teSparge->setCurrentFont(font);
    ui->teSparge->setTextColor(Qt::black);
    ui->teSparge->setText(SpargeTitle);

    string = QString(" 0            0.0 L %1.0 L |%2  %3 L").arg(pEbrew->mash_vol,3)
                                                            .arg(pEbrew->mlt2boil.size() ? pEbrew->mlt2boil.at(0) : "",9)
                                                            .arg(pEbrew->sp_vol_batch0,4,'f',1);
    if (spargeActive && !pEbrew->sp_idx)
    {
        font.setBold(true);
        ui->teSparge->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        font.setBold(false);
        ui->teSparge->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teSparge->setCurrentFont(font);
    ui->teSparge->append(string);
    int sptot = pEbrew->RegEbrew->value("SP_BATCHES").toInt();
    for (int i = 1; i <= sptot; i++)
    {
        string = QString("%1 %2 %3 L %4 L |%5  ").arg(i,2)
                                                 .arg(i < pEbrew->hlt2mlt.size() + 1 ? pEbrew->hlt2mlt.at(i-1) : "",9)
                                                 .arg(i * pEbrew->sp_vol_batch,4,'f',1)
                                                 .arg(i * pEbrew->sp_vol_batch + pEbrew->mash_vol,5,'f',1)
                                                 .arg(i < pEbrew->mlt2boil.size()    ? pEbrew->mlt2boil.at(i)  : "",9);
        if (i < sptot)
             string.append(QString("%1 L").arg(i * pEbrew->sp_vol_batch + pEbrew->sp_vol_batch0,4,'f',1));
        else string.append(QString("Empty MLT"));
        if (spargeActive && (i == pEbrew->sp_idx))
        {
            font.setBold(true);
            ui->teSparge->setTextColor(Qt::blue);
            string = "->" + string;
        } // if
        else
        {
            font.setBold(false);
            ui->teSparge->setTextColor(Qt::black);
            string = "  " + string;
        } // else
        ui->teSparge->setCurrentFont(font);
        ui->teSparge->append(string);
    } // for i

    // TIMERS
    bool timerActive;
    font.setBold(false);
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->setTextColor(Qt::black);
    int tmr1 = pEbrew->RegEbrew->value("SP_TIME").toInt() * 60;
    string = QString("Timer in state \'Sparge Timer Running\': %1/%2 sec.").arg(pEbrew->timer1,3).arg(tmr1,3);
    if (pEbrew->timer1 && (pEbrew->timer1 < tmr1))
    {
        font.setBold(true);
        ui->teTimers->setTextColor(Qt::blue);
        string      = "->" + string;
        timerActive = true;
    } // if
    else
    {
        font.setBold(false);
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
        timerActive = false;
    } // else
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->setText(string);
    string = QString("Timer in state \'Delay 10 seconds\'    : %1/%2 sec.").arg(pEbrew->timer2,3).arg(TMR_DELAY_xSEC,3);
    if (pEbrew->timer2 && (pEbrew->timer2 < TMR_DELAY_xSEC))
    {
        font.setBold(true);
        ui->teTimers->setTextColor(Qt::blue);
        string      = "->" + string;
        timerActive = true;
    } // if
    else
    {
        font.setBold(false);
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
        timerActive = false;
    } // else
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->append(string);
    string = QString("Timer in state \'Pump Pre-fill\'       : %1/%2 sec.").arg(pEbrew->timer3,3).arg(TMR_PREFILL_PUMP,3);
    if (pEbrew->timer3 && (pEbrew->timer3 < TMR_PREFILL_PUMP))
    {
        font.setBold(true);
        ui->teTimers->setTextColor(Qt::blue);
        string      = "->" + string;
        timerActive = true;
    } // if
    else
    {
        font.setBold(false);
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
        timerActive = false;
    } // else
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->append(string);
    string = QString("Timer in state \'Mash-rest 5 min.\'    : %1/%2 sec.").arg(pEbrew->mrest_tmr,3).arg(TMR_MASH_REST_5_MIN,3);
    if (pEbrew->mrest_tmr && (pEbrew->mrest_tmr < TMR_MASH_REST_5_MIN))
    {
        font.setBold(true);
        ui->teTimers->setTextColor(Qt::blue);
        string      = "->" + string;
        timerActive = true;
    } // if
    else
    {
        font.setBold(false);
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
        timerActive = false;
    } // else
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->append(string);
    string = QString("Timer in state \'Now Boiling\'         : %1/%2 min.\n").arg(pEbrew->timer5/60,3).arg(pEbrew->boil_time,3);
    if (pEbrew->timer5 && (pEbrew->timer5 < pEbrew->boil_time * 60))
    {
        font.setBold(true);
        ui->teTimers->setTextColor(Qt::blue);
        string      = "->" + string;
        timerActive = true;
    } // if
    else
    {
        font.setBold(false);
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
        timerActive = false;
    } // else
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->append(string);

    font.setBold(false);
    ui->teTimers->setCurrentFont(font);
    ui->teTimers->setTextColor(Qt::black);
    string = QString("  Boiling  started at ");
    if (pEbrew->Boil.size() > 0) string += pEbrew->Boil.at(0);
    if (pEbrew->Boil.size() > 1)
    {
        string += " and finished at ";
        string += pEbrew->Boil.at(1);
    } // if
    ui->teTimers->append(string);
    string = QString("  Chilling started at ");
    if (pEbrew->Chill.size() > 0) string += pEbrew->Chill.at(0);
    if (pEbrew->Chill.size() > 1)
    {
        string += " and finished at ";
        string += pEbrew->Chill.at(1);
    } // if
    ui->teTimers->append(string);

    if (timerActive || pEbrew->Boil.size() > 0)
    {
        ui->lblTimers->setEnabled(true);
        ui->teTimers->setEnabled(true);
    } // if
    else
    {
        ui->lblTimers->setEnabled(false);
        ui->teTimers->setEnabled(false);
    } // else
} // DialogViewProgress::onUpdateProgress()
