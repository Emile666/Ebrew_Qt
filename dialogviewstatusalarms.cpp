/**************************************************************************************
** Filename    : dialogviewstatusalarms.cpp
** Author      : Emile
** Purpose     : This file contains an information screen
**               for displaying various status and alarm signals.
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
#include "dialogviewstatusalarms.h"
#include "ui_dialogviewstatusalarms.h"

DialogViewStatusAlarms::DialogViewStatusAlarms(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogViewStatusAlarms)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);
    ui->setupUi(this);
    onUpdateProgress(); // start directly 1st time
    timerId = startTimer(2000); // update every 2 seconds
} // DialogViewStatusAlarms::DialogViewStatusAlarms()

DialogViewStatusAlarms::~DialogViewStatusAlarms()
{
    killTimer(timerId); // delete the timer
    delete ui;
} // DialogViewStatusAlarms::~DialogViewStatusAlarms()

void DialogViewStatusAlarms::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
        onUpdateProgress();
} // DialogViewProgress::timerEvent()

void DialogViewStatusAlarms::onUpdateProgress(void)
{
    QString string;
    QFont   font("Courier New",8);

    font.setBold(false);
    ui->teAlarms->setFont(font);
    ui->teAlarms->clear();
    //--------- THLT -------------
    string = QString("Thlt  (I2C) : ");
    if (pEbrew->thlt_i2c > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->thlt_i2c,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->setText(string);
    //--------- TMLT -------------
    string = QString("Tmlt  (I2C) : ");
    if (pEbrew->tmlt_i2c > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->tmlt_i2c,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->append(string);
    //--------- TCFC -------------
    string = QString("Tcfc  (OW)  : ");
    if (pEbrew->tcfc > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->tcfc,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->append(string);
    //--------- TBOIL -------------
    string = QString("Tboil (OW)  : ");
    if (pEbrew->tboil > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->tboil,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->append(string);
    //--------- THLT_OW -------------
    string = QString("Thlt  (OW)  : ");
    if (pEbrew->thlt_ow > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->thlt_ow,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->append(string);
    //--------- TMLT_OW -------------
    string = QString("Tmlt  (OW)  : ");
    if (pEbrew->tmlt_ow > SENSOR_VAL_LIM_OK)
    {
        string += QString("%1 °C").arg(pEbrew->tmlt_ow,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    ui->teAlarms->append(string);

    string = QString("Ttriac LM35 : ");
    if ((pEbrew->ttriac > SENSOR_VAL_LIM_OK) && !pEbrew->triacTooHot)
    {
        string += QString("%1 °C").arg(pEbrew->ttriac,2,'f',2);
        ui->teAlarms->setTextColor(Qt::black);
    } // if
    else
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // else
    if (pEbrew->triacTooHot) string += QString(" SSR too hot!");
    ui->teAlarms->append(string);
    //--------- FLOW1 -------------
    string = QString("\nFlow1 : ");
    if (pEbrew->sensorAlarmInfo & SENS_FLOW1)
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // if
    else
    {
        string += QString("%1 L,%2 L/min. std:%3 tmr:%4 lim:%5 L/min.").arg(pEbrew->FlowHltMlt,5,'f',1)
                                                                .arg(pEbrew->F1->getFlowRate(FLOWRATE_FIL),2,'f',1)
                                                                .arg(pEbrew->F1->frl_std,1)
                                                                .arg(pEbrew->F1->frl_tmr,2)
                                                                .arg(pEbrew->F1->frl_det_lim,1,'f',1);
        ui->teAlarms->setTextColor(Qt::black);
    } // else
    ui->teAlarms->append(string);
    //--------- FLOW2 -------------
    string = QString("Flow2 : ");
    if (pEbrew->sensorAlarmInfo & SENS_FLOW2)
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // if
    else
    {
        string += QString("%1 L,%2 L/min. std:%3 tmr:%4 lim:%5 L/min.")
                                                                .arg(pEbrew->FlowMltBoil,5,'f',1)
                                                                .arg(pEbrew->F2->getFlowRate(FLOWRATE_FIL),2,'f',1)
                                                                .arg(pEbrew->F2->frl_std,1)
                                                                .arg(pEbrew->F2->frl_tmr,2)
                                                                .arg(pEbrew->F2->frl_det_lim,1,'f',1);
        ui->teAlarms->setTextColor(Qt::black);
    } // else
    ui->teAlarms->append(string);
    //--------- FLOW3 -------------
    string = QString("Flow3 : ");
    if (pEbrew->sensorAlarmInfo & SENS_FLOW3)
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // if
    else
    {
        string += QString("%1 L,%2 L/min. std:%3 tmr:%4 lim:%5 L/min.")
                                                                .arg(pEbrew->FlowCfcOut,5,'f',1)
                                                                .arg(pEbrew->F3->getFlowRate(FLOWRATE_FIL),2,'f',1)
                                                                .arg(pEbrew->F3->frl_std,1)
                                                                .arg(pEbrew->F3->frl_tmr,2)
                                                                .arg(pEbrew->F3->frl_det_lim,1,'f',1);
        ui->teAlarms->setTextColor(Qt::black);
    } // else
    ui->teAlarms->append(string);
    //--------- FLOW4 -------------
    string = QString("Flow4 : ");
    if (pEbrew->sensorAlarmInfo & SENS_FLOW4)
    {
        string += QString("ERROR");
        ui->teAlarms->setTextColor(Qt::red);
    } // if
    else
    {
        string += QString("%1 L,%2 L/min. std:%3 tmr:%4 lim:%5 L/min.")
                                                                .arg(pEbrew->Flow4,5,'f',1)
                                                                .arg(pEbrew->F4->getFlowRate(FLOWRATE_FIL),2,'f',1)
                                                                .arg(pEbrew->F4->frl_std,1)
                                                                .arg(pEbrew->F4->frl_tmr,2)
                                                                .arg(pEbrew->F4->frl_det_lim,1,'f',1);
        ui->teAlarms->setTextColor(Qt::black);
    } // else
    ui->teAlarms->append(string);

    ui->tePidHlt->clear();
    ui->tePidHlt->setFont(font);
    string = QString("Status:");
    if (pEbrew->PidCtrlHlt->pid_on == PID_ON)
         string += "ON ";
    else if (pEbrew->PidCtrlHlt->pid_on == PID_FFC)
         string += "FFC";
    else string += "OFF";
    string += QString("\nLimits:%1 \% and %2 \%\n").arg(pEbrew->PidCtrlHlt->ykmin,1,'f',0)
                                                   .arg(pEbrew->PidCtrlHlt->ykmax,1,'f',0);
    ui->tePidHlt->append(string);
    qreal pp = pEbrew->PidCtrlHlt->pp;
    qreal pi = pEbrew->PidCtrlHlt->pi;
    qreal pd = pEbrew->PidCtrlHlt->pd;
    string = QString("P-action:%1\nI-action:%2\nD-action:%3\n").arg(pp,5,'f',1)
                                                               .arg(pi,5,'f',1)
                                                               .arg(pd,5,'f',1);
    string += QString("Δ y[k]  :%1\ny[k]    :%2").arg(pp+pi+pd,5,'f',1).arg(pEbrew->PidCtrlHlt->yk,5,'f',1);
    ui->tePidHlt->append(string);

    ui->tePidBk->clear();
    ui->tePidBk->setFont(font);
    string = QString("Status:");
    if (pEbrew->PidCtrlBk->pid_on == PID_ON)
         string += "ON ";
    else if (pEbrew->PidCtrlBk->pid_on == PID_FFC)
         string += "FFC";
    else string += "OFF";
    string += QString("\nLimits:%1 \% and %2 \%\n").arg(pEbrew->PidCtrlBk->ykmin,1,'f',0)
                                                   .arg(pEbrew->PidCtrlBk->ykmax,1,'f',0);
    ui->tePidBk->append(string);
    pp = pEbrew->PidCtrlBk->pp;
    pi = pEbrew->PidCtrlBk->pi;
    pd = pEbrew->PidCtrlBk->pd;
    string = QString("P-action:%1\nI-action:%2\nD-action:%3\n").arg(pp,5,'f',1)
                                                               .arg(pi,5,'f',1)
                                                               .arg(pd,5,'f',1);
    string += QString("Δ y[k]  :%1\ny[k]    :%2").arg(pp+pi+pd,5,'f',1).arg(pEbrew->PidCtrlBk->yk,5,'f',1);
    ui->tePidBk->append(string);
} // DialogViewStatusAlarms::onUpdateProgress()
