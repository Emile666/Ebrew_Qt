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

    ui->teMash->clear();
    if ((pEbrew->ms_idx < pEbrew->ms_tot) || (pEbrew->ms[pEbrew->ms_tot-1].timer < pEbrew->ms[pEbrew->ms_tot-1].time))
    {
        ui->teMash->setEnabled(true);
        ui->lblMash->setEnabled(true);
        ui->teMash->setText(MashTitle);
        //---------------------------------------------------------------------
        // Sample time of ms[] update is 1 second.
        // Time of ms[i].time was converted to seconds in readMashSchemeFile().
        //---------------------------------------------------------------------
        for (int i = 0; i < pEbrew->ms_tot; i++)
        {
            p = &pEbrew->ms[i];
            //---------------------------------------------------
            // Now update time_stamp if a mash-timer has started
            //---------------------------------------------------
            if ((p->timer != NOT_STARTED) && (p->time_stamp.isEmpty()))
            {
               p->time_stamp = QTime::currentTime().toString();
            } // if
            string = QString("%1 %2 %3 %4 %5  ").arg(i,2)
                                                .arg(p->temp ,5,'f',0)
                                                .arg(p->time ,5,'f',0)
                                                .arg(p->preht,5)
                                                .arg(p->temp ,5,'f',0);
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
            if (i == pEbrew->ms_idx)
            {
                ui->teMash->setTextColor(Qt::blue);
                string = "->" + string;
            } // if
            else
            {
                ui->teMash->setTextColor(Qt::black);
                string = "  " + string;
            } // else
            ui->teMash->append(string);
        } // for i
    } // if
    else
    {   // mashing not active anymore
        ui->teMash->setEnabled(false);
        ui->lblMash->setEnabled(false);
    } // else

    // SPARGE PROGRESS
    //if ((pEbrew->ebrew_std >= S05_SPARGE_TIMER_RUNNING) && (pEbrew->ebrew_std < S09_EMPTY_MLT))
    if (true)
    {
        ui->teSparge->setEnabled(true);
        ui->lblSparge->setEnabled(true);
        ui->teSparge->setText(SpargeTitle);
        if ((pEbrew->ebrew_std == S06_PUMP_FROM_MLT_TO_BOIL) && (prev_ebrew_std == S05_SPARGE_TIMER_RUNNING))
        {  // New transition detected, copy time-stamp into array of strings
           mlt2boil << QTime::currentTime().toString();
        } // if
        if ((pEbrew->ebrew_std == S07_PUMP_FROM_HLT_TO_MLT) && ((prev_ebrew_std == S08_DELAY_xSEC) || (prev_ebrew_std == S06_PUMP_FROM_MLT_TO_BOIL)))
        {  // New transition detected, copy time-stamp into array of strings
           hlt2mlt << QTime::currentTime().toString();
        } // if
        if ((pEbrew->ebrew_std == S11_BOILING) && (prev_ebrew_std == S10_WAIT_FOR_BOIL))
        {  // New transition detected, copy time-stamp into array of strings
           Boil << QTime::currentTime().toString();
        } // if
        if ((pEbrew->ebrew_std == S12_BOILING_FINISHED) && (prev_ebrew_std == S11_BOILING))
        {  // New transition detected, copy time-stamp into array of strings
           Boil << QTime::currentTime().toString();
        } // if
        if ((pEbrew->ebrew_std == S16_CHILL_PUMP_FERMENTOR) && (prev_ebrew_std == S12_BOILING_FINISHED))
        {  // New transition detected, copy time-stamp into array of strings
           Chill << QTime::currentTime().toString();
        } // if
        if ((pEbrew->ebrew_std == S17_FINISHED) && (prev_ebrew_std == S16_CHILL_PUMP_FERMENTOR))
        {  // New transition detected, copy time-stamp into array of strings
           Chill << QTime::currentTime().toString();
        } // if
        prev_ebrew_std = pEbrew->ebrew_std; // update previous value

        string = QString("  0    - - - - - - - -  |%1 %2 L").arg(mlt2boil.size() ? mlt2boil.at(0) : "",11).arg(pEbrew->sp_vol_batch0,4,'f',1);
        if (!pEbrew->sp_idx)
        {
            ui->teSparge->setTextColor(Qt::blue);
            string = "->" + string;
        } // if
        else
        {
            ui->teSparge->setTextColor(Qt::black);
            string = "  " + string;
        } // else
        ui->teSparge->append(string);
        int sptot = pEbrew->RegEbrew->value("SP_BATCHES").toInt();
        for (int i = 1; i <= sptot; i++)
        {
            string = QString("%1 %2 %3 L  |%4 ").arg(i,3)
                                                  .arg(i < hlt2mlt.size() ? hlt2mlt.at(i) : "",11)
                                                  .arg(i * pEbrew->sp_vol_batch,4,'f',1)
                                                  .arg(i < mlt2boil.size() ? mlt2boil.at(i) : "",11);
            if (i < sptot)
                 string.append(QString("%1 L").arg(i * pEbrew->sp_vol_batch + pEbrew->sp_vol_batch0,4,'f',1));
            else string.append(QString("Empty MLT"));
            if (i == pEbrew->sp_idx)
            {
                ui->teSparge->setTextColor(Qt::blue);
                string = "->" + string;
            } // if
            else
            {
                ui->teSparge->setTextColor(Qt::black);
                string = "  " + string;
            } // else
            ui->teSparge->append(string);
        } // for i
    } // if
    else
    {   // Sparging not active
        ui->teSparge->setEnabled(false);
        ui->lblSparge->setEnabled(false);
    } // else

    // TIMERS
    string = QString("Timer in state \'Sparge Timer Running\': %1/%2 min.").arg(pEbrew->timer1/60,3).arg(pEbrew->RegEbrew->value("SP_TIME").toInt(),3);
    if (pEbrew->timer1)
    {
        ui->teTimers->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teTimers->setText(string);
    string = QString("Timer in state \'Delay 10 seconds\'    : %1/%2 sec.").arg(pEbrew->timer2,3).arg(TMR_DELAY_xSEC,3);
    if (pEbrew->timer2)
    {
        ui->teTimers->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teTimers->append(string);
    string = QString("Timer in state \'Pump Pre-fill\'       : %1/%2 sec.").arg(pEbrew->timer3,3).arg(TMR_PREFILL_PUMP,3);
    if (pEbrew->timer3)
    {
        ui->teTimers->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teTimers->append(string);
    string = QString("Timer in state \'Mash-rest 5 min.\'    : %1/%2 sec.").arg(pEbrew->mrest_tmr,3).arg(TMR_MASH_REST_5_MIN,3);
    if (pEbrew->mrest_tmr)
    {
        ui->teTimers->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teTimers->append(string);
    string = QString("Timer in state \'Now Boiling\'         : %1/%2 min.\n").arg(pEbrew->timer5/60,3).arg(pEbrew->boil_time,3);
    if (pEbrew->timer5)
    {
        ui->teTimers->setTextColor(Qt::blue);
        string = "->" + string;
    } // if
    else
    {
        ui->teTimers->setTextColor(Qt::black);
        string = "  " + string;
    } // else
    ui->teTimers->append(string);

    string = QString("  Boiling started at  : ");
    if (Boil.size() > 0) string += Boil.at(0);
    ui->teTimers->append(string);
    string = QString("  Boiling finished at : ");
    if (Boil.size() > 1) string += Boil.at(1);
    ui->teTimers->append(string);

} // DialogViewProgress::onUpdateProgress()
