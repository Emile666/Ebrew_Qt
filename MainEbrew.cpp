/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include "MainEbrew.h"
#include "dialogeditmashscheme.h"
#include "dialogeditfixparameters.h"
#include "dialogoptionspidsettings.h"
#include "dialogoptionsmeasurements.h"
#include "dialogbrewdaysettings.h"
#include "dialogoptionssystemsettings.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QDebug>
#include <QFile>
#include <QLabel>

//------------------------------------------------------------------------------------------
MainEbrew::MainEbrew(void) : QMainWindow()
{
    SlopeLimiter *pSlopeLim; // pointer to SlopeLimiter object

    createStatusBar(); // create status bar at the bottom of the screen
    createMenuBar();   // create menu bar at the top of the screen
    RegEbrew = new QSettings(REGKEY,QSettings::NativeFormat);
    if (!RegEbrew->contains("BOIL_DETECT"))
    {   // Windows registry does not exist yet
        createRegistry(); // create default Registry entries
        qDebug() << "ebrew Registry was NOT found, init. Registry with default values";
    } // if
    readMashSchemeFile(INIT_TIMERS); // Read mash scheme from file and init. all mash timers

    pSlopeLim = new SlopeLimiter();
    pSlopeLim->setLim(RegEbrew->value("TSET_SLOPE_LIM").toDouble());
    slopeLimHLT = pSlopeLim; // copy pointer to MainEbrew
    pSlopeLim  = new SlopeLimiter();
    pSlopeLim->setLim(RegEbrew->value("TSET_SLOPE_LIM").toDouble());
    slopeLimBK = pSlopeLim; // copy pointer to MainEbrew
} // MainEbrew::MainEbrew()

/*------------------------------------------------------------------
  Purpose  : This function creates the status bar at the bottom of
             the screen and the toolbar at the top of the screen.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::createStatusBar(void)
{
    auto statusBar = new QStatusBar;  // Statusbar

    //-----------------------------------
    // Statusbar at bottom of screen
    //-----------------------------------
    statusAlarm      = new QLabel(" Sensor Alarm: TEMP + FLOW ");
    statusAlarm->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusAlarm);
    statusMashScheme = new QLabel(" 44°C(50 min.),52°C(15 min.),63°C(60 min.),73°C(20 min.),78°C(5 min.) ");
    statusMashScheme->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusMashScheme);
    statusMashVol    = new QLabel(" Mash Volume: 85 L ");
    statusMashVol->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusMashVol);
    statusSpargeVol  = new QLabel(" Sparge Volume: 42 L ");
    statusSpargeVol->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusSpargeVol);
    statusBoilTime  = new QLabel(" Boil-time: 120 min. ");
    statusBoilTime->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusBoilTime);
    statusMsIdx      = new QLabel(" Mash index: 0 ");
    statusMsIdx->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusMsIdx);
    statusSpIdx      = new QLabel(" Sparge index: 0 ");
    statusSpIdx->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusSpIdx);
    statusSwRev      = new QLabel(" SW r1.99 HW r1.24 ");
    statusSwRev->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusSwRev);
    setStatusBar(statusBar); // connect the statusBar to Ebrew

    // Create Toolbar at top of screen
    auto toolBar = new QToolBar("Toolbar"); // Toolbar at left of screen
    toolBar->setMovable(false); // fixed at top of screen
    toolBar->setOrientation(Qt::Horizontal);
    toolStartCIP = new QCheckBox("Start Clean-In-Place (CIP) Program");
    toolBar->addWidget(toolStartCIP);
    toolStartAddMalt = new QCheckBox("Start Adding Malt");
    toolStartAddMalt->setEnabled(false); // default not enabled
    toolBar->addWidget(toolStartAddMalt);
    toolMaltAdded = new QCheckBox("Malt added to MLT");
    toolMaltAdded->setEnabled(false); // default not enabled
    toolBar->addWidget(toolMaltAdded);
    toolBoilStarted = new QCheckBox("Boiling Started");
    toolBoilStarted->setEnabled(false); // default not enabled
    toolBar->addWidget(toolBoilStarted);
    toolStartChilling = new QCheckBox("CFC Prepared, start Chilling");
    toolStartChilling->setEnabled(false); // default not enabled
    toolBar->addWidget(toolStartChilling);
    addToolBar(Qt::TopToolBarArea,toolBar);
} // MainEbrew::createStatusBar()

/*------------------------------------------------------------------
  Purpose  : This function creates the menu bar at the top of the screen
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::createMenuBar(void)
{
    auto menuBar = new QMenuBar;    // See Examples\Qt-5.14.2\network\torrent

    // File menu
    auto Fmenu       = new QMenu("&File");
    Fmenu->addAction(QIcon(":/img/fileopen.png"),"Read Log-File..."); // TODO
    Fmenu->addSeparator();
    Fmenu->addAction(QIcon(":/img/exit.png")    , "E&xit", this,SLOT(close()));
    menuBar->addMenu(Fmenu);
    // Edit menu
    auto Emenu       = new QMenu("&Edit");
    Emenu->addAction(QIcon(":/img/fileedit.png") ,"&Mash Scheme..."   ,this,SLOT(MenuEditMashScheme()));
    Emenu->addAction(QIcon(":/img/fixparams.png"),"&Fix Parameters...",this,SLOT(MenuEditFixParameters()));
    menuBar->addMenu(Emenu);
    // View menu
    auto Vmenu       = new QMenu("&View");
    Vmenu->addAction(QIcon(":/img/progress.png"),"&Mash && Sparge Progress"); // TODO
    Vmenu->addAction(QIcon(":/img/alarm.png")   ,"&Status and Alarms");          // TODO
    Vmenu->addAction(QIcon(":/img/task.png")    ,"&Task-list and Timings");       // TODO
    menuBar->addMenu(Vmenu);
    // Options menu
    auto Omenu       = new QMenu("&Options");
    Omenu->addAction(QIcon(":/img/hwsettings.png")  ,"&System Settings..."        ,this,SLOT(MenuOptionsSystemSettings()));
    Omenu->addAction(QIcon(":/img/pidsettings.png") ,"&PID Controller Settings...",this,SLOT(MenuOptionsPidSettings()));
    Omenu->addAction(QIcon(":/img/cooking.png")     ,"Brew Day Settings..."       ,this,SLOT(MenuOptionsBrewDaySettings()));
    Omenu->addAction(QIcon(":/img/measurements.png"),"Measurements Settings..."   ,this,SLOT(MenuOptionsMeasurements()));
    menuBar->addMenu(Omenu);
    // Help menu
    auto Hmenu       = new QMenu("&Help");
    Hmenu->addAction(QIcon(":/img/about.png"),"&About"   ,this,SLOT(about()));
    Hmenu->addAction(QIcon(":/img/qt.png")   ,"About &Qt",qApp,SLOT(aboutQt()));
    menuBar->addMenu(Hmenu);
    setMenuBar(menuBar);    // Connect the menuBar to Ebrew
} // MainEbrew::createMenuBar()

/*------------------------------------------------------------------
  Purpose  : This function creates default entries in the Registry.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::createRegistry(void)
{
    //------------------------------------
    // Options -> System Settings Dialog
    //------------------------------------
    // Heating mode
    RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",2);
    RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",4);
    RegEbrew->setValue("GAS_NON_MOD_LLIMIT",30);
    RegEbrew->setValue("GAS_NON_MOD_HLIMIT",35);
    RegEbrew->setValue("TTRIAC_LLIM",65);
    RegEbrew->setValue("TTRIAC_HLIM",75);
    // Communications
    RegEbrew->setValue("SYSTEM_MODE",GAS_MODULATING);
    RegEbrew->setValue("COMM_CHANNEL",0);  // Select Ethernet as Comm. Channel
    RegEbrew->setValue("COM_PORT_SETTINGS","38400,N,8,1"); // COM port settings
    RegEbrew->setValue("UDP_IP_PORT","192.168.192.105:8888"); // IP & Port number
    RegEbrew->setValue("CB_DEBUG_COM_PORT",1);
    RegEbrew->setValue("CB_SHOW_SENSOR_INFO",1); // TODO
    // Brew-kettle Sizes
    RegEbrew->setValue("VHLT_MAX",200);       // Max. HLT volume
    RegEbrew->setValue("VMLT_MAX",110);       // Max. MLT volume
    RegEbrew->setValue("VBOIL_MAX",140);      // Max. Boil kettle volume

    //------------------------------------
    // Options -> PID Settings Dialog
    //------------------------------------
    RegEbrew->setValue("TS",20.0);            // Set Default sample time
    RegEbrew->setValue("Kc",80.0);            // Controller gain
    RegEbrew->setValue("Ti",282.0);           // Ti constant
    RegEbrew->setValue("Td",20.0);            // Td constant
    RegEbrew->setValue("TSET_SLOPE_LIM",1.0); // Slope Limit for Temperature Setpoints

    //------------------------------------
    // Options -> Brew Day Settings Dialog
    //------------------------------------
    // Sparge Settings
    RegEbrew->setValue("SP_BATCHES",5);      // #Sparge Batches
    RegEbrew->setValue("SP_TIME",12);        // Time between sparge batches
    RegEbrew->setValue("SP_MPY",2.0);        // Multiply factor for 1st run-off volume to Boil-Kettle
    // Mash Settings
    //RegEbrew->setValue("ms_idx",MAX_MS);     // init. index in mash scheme TODO
    RegEbrew->setValue("TOffset0",3.5);      // Compensation for dough-in of malt
    RegEbrew->setValue("TOffset",1.0);       // Compensation HLT-MLT heat-loss
    RegEbrew->setValue("TOffset2",-0.5);     // Early start of mash-timer
    RegEbrew->setValue("PREHEAT_TIME",15);   // PREHEAT_TIME [min.]
    RegEbrew->setValue("CB_Malt_First",0);   // 1= Add malt first to MLT, then add water
    RegEbrew->setValue("CB_Mash_Rest",1);    // Mash Rest for 5 minutes after Malt is added
    RegEbrew->setValue("CB_dpht",1);         // 1= use dynamic preheat timing
    RegEbrew->setValue("HLT_Bcap",90);       // HLT burner capacity in sec./°C
    RegEbrew->setValue("CB_pumps_on",1);     // 1= Leave pumps runnings after MLT temp. is reached
    // Boil Settings
    RegEbrew->setValue("BOIL_MIN_TEMP",60);  // Min. Temp. for Boil-Kettle (Celsius)
    RegEbrew->setValue("SP_PREBOIL",95);     // Pre-Boil Temperature (Celsius)
    RegEbrew->setValue("BOIL_DETECT",99.3);  // Boiling-Detection minimum Temperature (Celsius)
    RegEbrew->setValue("SP_BOIL",105);       // Boil Temperature (Celsius)
    RegEbrew->setValue("LIMIT_BOIL",100);    // Limit output during boil (%)
    RegEbrew->setValue("CB_Boil_Rest",1);    // Let wort rest for 5 minutes after boiling
    // Clean in Place (CIP) Settings
    RegEbrew->setValue("CIP_SP",65);         // CIP Temperature Setpoint (Celsius)
    RegEbrew->setValue("CIP_CIRC_TIME",300); // CIP Circulating time (seconds)
    RegEbrew->setValue("CIP_REST_TIME",300); // CIP Rest time (seconds)
    RegEbrew->setValue("CIP_OUT_TIME",300);  // CIP Circulating time for cleaning outputs (seconds)
    RegEbrew->setValue("CIP_INP_TIME",300);  // CIP Circulating time for cleaning inputs (seconds)

    //------------------------------------
    // Options -> Measurements Dialog
    //------------------------------------
    // Temperatures
    RegEbrew->setValue("THLT_OFFSET",0.0);    // Offset for Thlt
    RegEbrew->setValue("TMLT_OFFSET",0.0);    // Offset for Tmlt
    RegEbrew->setValue("TBOIL_OFFSET",0.0);   // Offset for Tboil
    RegEbrew->setValue("TCFC_OFFSET",0.0);    // Offset for Tcfc
    // Flows
    RegEbrew->setValue("FLOW1_ERR",0);        // Error Correction for FLOW1
    RegEbrew->setValue("FLOW2_ERR",0);        // Error Correction for FLOW2
    RegEbrew->setValue("FLOW3_ERR",0);        // Error Correction for FLOW3
    RegEbrew->setValue("FLOW4_ERR",0);        // Error Correction for FLOW4
    RegEbrew->setValue("FLOW_TEMP_CORR",1);   // Use Temperature Correction
    RegEbrew->setValue("MIN_FR_MLT_PERC",10); // Min. Flowrate for MLT Empty detection
    RegEbrew->setValue("MIN_FR_BOIL_PERC",2); // Min. Flowrate for Boil-Kettle Empty detection
} // createRegistry()

/*------------------------------------------------------------------
  Purpose  : This function sets the kettle names using the volumes
             found in the Registry.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::setKettleNames(void)
{
    hlt->setName(QString("HLT %1 L").arg(RegEbrew->value("VHLT_MAX").toInt()));
    mlt->setName(QString("MLT %1 L").arg(RegEbrew->value("VMLT_MAX").toInt()));
    boil->setName(QString("BOIL %1 L").arg(RegEbrew->value("VBOIL_MAX").toInt()));
} // MainEbrew::SetKettleNames()

/*------------------------------------------------------------------
  Purpose  : This function is the main-entry point for all time-related
             functions. It is called every 100 msec. from a QTimer in
             a separate thread.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::T100msecLoop(void)
{
    thlt += 0.01;
    hlt->setValues(thlt,tset_hlt,Vhlt,gamma_hlt); // temp, sp, vol, power
    hlt->update();
} // MainEbrew::T100msecLoop()

/*------------------------------------------------------------------
  Purpose  : This function reads a Mash Scheme from a .sch file and
             updates the statusbar with the new temp-time pairs.
  Variables: ms[] array is filled with new temp-time pairs
             ms_tot is updated (total number of temp-time pairs)
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::readMashSchemeFile(bool initTimers)
{
    QFile       inputFile(MASHFILE);
    QString     line,sbar;
    QStringList list1;
    int i = 0;

    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while ((i++ < 3) && !in.atEnd())
       {  // read 3 dummy lines
          line = in.readLine();
       } // while
       line = in.readLine(); // Read mash water volume
       list1 = line.split(':');
       if (list1.size() >= 2)
            mash_vol = list1.at(1).toInt();
       else mash_vol = 0; // error in maisch.sch
       sbar.clear();
       sbar = QString(" Mash: %1 L ").arg(mash_vol);
       statusMashVol->setText(sbar);
       line = in.readLine(); // Read sparge water volume
       list1 = line.split(':');
       if (list1.size() >= 2)
            sp_vol = list1.at(1).toInt();
       else sp_vol = 0; // error in maisch.sch
       sbar.clear();
       sbar = QString(" Sparge: %1 L ").arg(sp_vol);
       statusSpargeVol->setText(sbar);
       line = in.readLine(); // Read boiling-time
       list1 = line.split(':');
       if (list1.size() >= 2)
            boil_time = list1.at(1).toInt();
       else boil_time = 0; // error in maisch.sch
       sbar.clear();
       sbar = QString(" Boil: %1 min. ").arg(boil_time);
       statusBoilTime->setText(sbar);
       i = 0;
       while ((i++ < 3) && !in.atEnd())
       {  // read 3 dummy lines
          line = in.readLine();
       } // while
       ms_tot = 0;
       sbar.clear(); // clear QString for statusbar
       while ((ms_tot < MAX_MS) && !in.atEnd())
       {  // read line
          line = in.readLine();
          list1 = line.split(',');
          if (list1.size() >= 2)
          {
              ms[ms_tot].temp = list1.at(0).toDouble();
              ms[ms_tot].time = 60.0 * list1.at(1).toDouble(); // from min. to sec.
              if (initTimers)
              {
                 ms[ms_tot].timer         = NOT_STARTED; /* init. timer to not started */
                 ms[ms_tot].time_stamp[0] = '\0';        /* init. time-stamp to empty string */
              } // if
              if (!sbar.isEmpty()) sbar.append(", ");
              sbar.append(QString("%1 °C(%2 min.)").arg(ms[ms_tot].temp,2,'f',0).arg(ms[ms_tot].time/60.0,2,'f',0));
              ms_tot++; // number of total temperature-time pairs
          } // if
       } // while
       statusMashScheme->setText(sbar);
       inputFile.close();
    }  // if
} // MainEbrew::readMashSchemeFile()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: ALIVE Led toggle
  Period-Time: 0.5 second
  Variables  : -
  Returns    : -
  ---------------------------------------------------------------------------*/
void MainEbrew::task_alive_led(void)
{
    char   s[20];

   //------------------------------------------------------------------
   // Toggle alive toggle bit to see if this routine is still alive
   //------------------------------------------------------------------
   if (toggle_led)
        strcpy(s,"L1\n");
   else strcpy(s,"L0\n");
   toggle_led = !toggle_led;
   tmlt += 0.1;
   mlt->setValues(tmlt,0.0,0.0,0.0);
   mlt->update();
   //MainForm->comm_port_write(s); // Send L1/L0 command to ebrew hardware
} // task_alive_led()

/*------------------------------------------------------------------
  Purpose  : This function is called every seconds from the scheduler.
             It calles the Ebrew State Transition Diagram (STD).
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::task_update_std(void)
{
    uint16_t std_out;     // values of valves
    uint8_t  pump_bits;   // values of pumps

    std_out = state_machine(); // call the Ebrew STD
    setStateName();            // update std text on screen
    if (tset_hlt_sw)
    {  // Set Temperature Setpoint for HLT to a fixed value
       tset_hlt = tset_hlt_fx; // fix tset_hlt
    } // if
    tset_hlt = slopeLimHLT->slopeLimit(tset_hlt);
    if (tset_boil_sw)
    {  // Set Temperature Setpoint for Boil-Kettle to a fixed value
       tset_boil = tset_boil_fx;
    } // if
    tset_boil = slopeLimBK->slopeLimit(tset_boil);

    if (hlt_pid->isChecked() && (tset_hlt < 5.0)) // TODO: move to state_machine()
    {   // Disable PID controller when sparging is finished
        hlt_pid->setChecked(false);
    } // if
    //-----------------------------------------------------------------
    // Now output all valve bits to Ebrew hardware.
    // NOTE: The pump bit is sent using the P0/P1 command
    //-----------------------------------------------------------------
    //sprintf(s,"V%d\n",(std_out & ALL_VALVES)); // Output all valves
    //MainForm->comm_port_write(s); // output Vxxx to Ebrew hardware TODO

    //--------------------------------------------
    // Send Pump On/Off signals to ebrew hardware.
    //--------------------------------------------
    if (std_out & P0b) pump_bits  = 0x01;
    else               pump_bits  = 0x00;
    if (std_out & P1b) pump_bits |= 0x02;
    //sprintf(s,"P%d\n",pump_bits);
    //MainForm->comm_port_write(s); // Send Px command to ebrew hardware TODO
} // MainEbrew::task_update_std()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: Read all 5 Temperature values from hardware
  Period-Time: 2 seconds
  Variables: -
  Returns  : -
  ---------------------------------------------------------------------------*/
void MainEbrew::task_read_temps(void)
{
    double temp1 = 0.0, temp2 = 0.0, temp3 = 0.0, temp4 = 0.0, temp5 = 0.0;

    //------------------ TEMP1 (LM35) -----------------------------------------
    ttriac = temp1;
    if (ttriac_sw)
    {  // Switch & Fix
       ttriac = ttriac_fx;
    } // if
    //---------------------------------------------------
    // Triac Temperature Protection: hysteresis function
    //---------------------------------------------------
    if (triac_too_hot)
    { // Reset if temp. < lower-limit
      triac_too_hot = (ttriac >= RegEbrew->value("TTRIAC_LLIM").toInt());
    } // if
    else
    { // set if temp. >= upper-limit
      triac_too_hot = (ttriac >= RegEbrew->value("TTRIAC_HLIM").toInt());
    } // else
    //------------------ TEMP2 (THLT) -----------------------------------------
    if (temp2 > SENSOR_VAL_LIM_OK)
    {
         //MainForm->Val_Thlt->Font->Color = clLime;
         thlt = temp2 + thlt_offset; // update THLT with new value
         //MainForm->sensor_alarm_info &= ~SENS_THLT;      // reset bit in sensor_alarm
    } // if
//    else
//    {
//         MainForm->Val_Thlt->Font->Color = clRed; // + do NOT update THLT
//         if ((MainForm->no_sound == ALARM_TEMP_SENSORS) || (MainForm->no_sound == ALARM_TEMP_FLOW_SENSORS))
//         {
//              MainForm->comm_port_write("X3\n");
//              MainForm->sensor_alarm_info |= SENS_THLT;
//         } // if
//    } // else
    if (thlt_sw)
    {  // Switch & Fix
       thlt = thlt_fx;
    } // if
    //------------------ TEMP3 (TMLT) -----------------------------------------
    if (temp3 > SENSOR_VAL_LIM_OK)
    {
         //MainForm->Val_Tmlt->Font->Color = clLime;
         tmlt = temp3 + tmlt_offset; // update TMLT with new value
         //MainForm->sensor_alarm_info &= ~SENS_TMLT;      // reset bit in sensor_alarm
    } // if
//    else
//    {
//        MainForm->Val_Tmlt->Font->Color = clRed; // + do NOT update TMLT
//        if ((MainForm->no_sound == ALARM_TEMP_SENSORS) || (MainForm->no_sound == ALARM_TEMP_FLOW_SENSORS))
//        {
//             MainForm->comm_port_write("X3\n");
//             MainForm->sensor_alarm_info |= SENS_TMLT;
//        } // if
//    } // else
    if (tmlt_sw)
    {  // Switch & Fix
       tmlt = tmlt_fx;
    } // if
    //------------------ TEMP4 (TBOIL) ----------------------------------------
    if (temp4 > SENSOR_VAL_LIM_OK)
    {
         //MainForm->Temp_Boil->Font->Color = clLime;
         tboil = temp4 + tboil_offset; // update TBOIL with new value
         //MainForm->sensor_alarm_info &= ~SENS_TBOIL;       // reset bit in sensor_alarm
    } // if
//    else
//    {
//         MainForm->Temp_Boil->Font->Color = clRed; // + do NOT update TBOIL
//         if ((MainForm->no_sound == ALARM_TEMP_SENSORS) || (MainForm->no_sound == ALARM_TEMP_FLOW_SENSORS))
//         {
//              MainForm->comm_port_write("X3\n");
//              MainForm->sensor_alarm_info |= SENS_TBOIL;
//         } // if
//    } // else
    if (tboil_sw)
    {  // Switch & Fix
       tboil = tboil_fx;
    } // if
    //------------------ TEMP5 (TCFC) -----------------------------------------
    if (temp5 > SENSOR_VAL_LIM_OK)
    {
         //MainForm->Temp_CFC->Font->Color = clLime;
         tcfc = temp5 + tcfc_offset; // update TCFC with new value
         //MainForm->sensor_alarm_info &= ~SENS_TCFC;      // reset bit in sensor_alarm
    } // if
//    else
//    {
//         MainForm->Temp_CFC->Font->Color = clRed; // + do NOT update TCFC
//         if ((MainForm->no_sound == ALARM_TEMP_SENSORS) || (MainForm->no_sound == ALARM_TEMP_FLOW_SENSORS))
//         {
//              MainForm->comm_port_write("X3\n");
//              MainForm->sensor_alarm_info |= SENS_TCFC;
//         } // if
//    } // else
    // No switch/fix needed for TCFC
} // MainEbrew::task_read_temps()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: Read all 4 flow sensors from hardware
  Period-Time: 2 seconds
  Variables: -
  Returns  : -
  ---------------------------------------------------------------------------*/
void MainEbrew::task_read_flows(void)
{
    double err, temp;

    //------------------ FLOW1 ------------------------------------------------
    if (flow_temp_corr) // TODO
    {   // Apply correction for increased volume at higher temperatures
        err = (1.0 + 0.00021 * (thlt - 20.0));
        Flow_hlt_mlt /= err;
    } // if
    // Now apply Calibration error correction
    Flow_hlt_mlt *= 1.0 + 0.01 * flow1_err; // TODO
    // Calculate Flow-rate in L per minute ; this task is called once per 2 seconds
    temp = 30.0 * (Flow_hlt_mlt - Flow_hlt_mlt_old);
//    if ((temp < 0.1) && flow1_running &&
//        ((no_sound == ALARM_FLOW_SENSORS) || (no_sound == ALARM_TEMP_FLOW_SENSORS)))
//    {
//         MainForm->comm_port_write("X2\n"); // sound alarm
//         MainForm->sensor_alarm_info |=  SENS_FLOW1;
//    } // if
//    else MainForm->sensor_alarm_info &= ~SENS_FLOW1;
    //Flow_rate_hlt_mlt = moving_average(&MainForm->flow1_ma,temp); TODO
    // Calculate VHLT volume here
    //Vhlt = vhlt_max - Flow_hlt_mlt;
    if (vhlt_sw)
    {  // Switch & Fix
       Vhlt = vhlt_fx;
    } // if

} // task_read_flows()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Help->About Ebrew is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::about(void)
{
   QMessageBox::about(this,"About Ebrew 3.0 Qt",
                "This program is used to fully control a HERMS system. "
                "It communicates with the <b>Ebrew hardware</b>, which is a"
                " dedicated hardware solution, based around an Arduino Nano<br><br>"
                "This version is a redesign from Ebrew 2.0, that was created"
                " with Borland C++ Builder and was in use from 2003 - 2020. It's latest revision was r1.99.<br><br>"
                "This version (Ebrew 3.0 Qt) is a complete redesign and is built with Qt 5.<br><br>"
                "Web-site: <a href=\"www.vandelogt.nl\">www.vandelogt.nl</a><br>"
                "Brewery: Brouwerij de Boezem, The Netherlands");
} // MainEbrew::about()

/*------------------------------------------------------------------
  Purpose  : This function updates the std_text on the screen with
             the new text and subtext for the STD state.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::setStateName(void)
{
    QString string;

    switch (ebrew_std)
    {
        case S00_INITIALISATION:
             string = QString("00. Initialisation");
             std_text->setSubText("Press the HLT PID Powerbutton to advance to the next state");
             break;
        case S01_WAIT_FOR_HLT_TEMP:
             string = QString("01. Wait for HLT Temp. (%1 °C)").arg(tset_hlt,2,'f',1);
             std_text->setSubText("The HLT temperature is heated to the first temperature of the mash scheme");
             if (malt_first && !toolMaltAdded->isChecked())
             {
                   string.append(" + Add Malt to MLT (M)");
                   std_text->setSubText("If malt is added, click \'Malt is Added\' on toolbar at top of screen");
             } // if
             break;
        case S02_FILL_MLT:
             string = QString("02. Fill MLT with %1 L water").arg(mash_vol);
             std_text->setSubText("The MLT needs to be filled with the required amount of mash water");
             break;
        case S03_WAIT_FOR_MLT_TEMP:
             string = QString("03. Wait for MLT Temperature (%1 °C)").arg(ms[ms_idx].temp + temp_offset2,2,'f',1);
             std_text->setSubText("If the MLT Temperature is correct, the mashing phase is started");
             break;
        case S04_MASH_TIMER_RUNNING:
             string = QString("04. Mash-Timer Running (%1/%2 min.)").arg(ms[ms_idx].timer/60).arg(ms[ms_idx].time/60.0,2,'f',1);
             std_text->setSubText("After timeout, HLT is heated to the next temperature");
             break;
        case S05_SPARGE_TIMER_RUNNING:
             string = QString("05. Sparge-Timer Running (%d/%d min.)").arg(timer1/60).arg(sp_time);
             std_text->setSubText("After timeout, wort is pumped to the boil-kettle");
             break;
//        case S06_PUMP_FROM_MLT_TO_BOIL:
//             sprintf(tmp_str,"06. Pump from MLT to Boil-Kettle (%2.1f L)",std.sp_idx ? sp.sp_vol_batch : sp.sp_vol_batch0);
//             Std_State->Caption = tmp_str;
//             break;
//        case S07_PUMP_FROM_HLT_TO_MLT:
//             sprintf(tmp_str,"07. Pump fresh water from HLT to MLT (%2.1f L)",sp.sp_vol_batch);
//             Std_State->Caption = tmp_str;
//             break;
//        case S08_DELAY_xSEC:
//             sprintf(tmp_str,"08. Delay: %d seconds",TMR_DELAY_xSEC);
//             Std_State->Caption = tmp_str;
//             break;
//        case S09_EMPTY_MLT:
//             Std_State->Caption = "09. Empty MLT";
//             break;
//        case S10_WAIT_FOR_BOIL:
//             Std_State->Caption = "10. Wait for Boil (M)";
//             break;
//        case S11_BOILING:
//             sprintf(tmp_str,"11. Now Boiling (%d/%d min.)",std.timer5/60,sp.boil_time);
//             Std_State->Caption = tmp_str;
//             break;
//        case S12_BOILING_FINISHED:
//             if (std.boil_rest)
//             {
//                if (std.brest_tmr > TMR_BOIL_REST_5_MIN)
//                     sprintf(tmp_str,"12. Boiling Finished, prepare Chiller (M)");
//                else sprintf(tmp_str,"12. Boiling Finished, wait %d/%d min., prepare Chiller (M)",std.brest_tmr/60,TMR_BOIL_REST_5_MIN/60);
//                Std_State->Caption = tmp_str;
//             } // if
//             else Std_State->Caption = "12. Boiling Finished, prepare Chiller (M)";
//             break;
//        case S13_MASH_PREHEAT_HLT:
//             sprintf(tmp_str,"13. Mash Preheat HLT (%d/%2.0f min.)",ms[std.ms_idx].timer/60,ms[std.ms_idx].time/60.0);
//             Std_State->Caption = tmp_str;
//             break;
        case S14_PUMP_PREFILL:
             string = QString("14. Pump Pre-Fill / Priming (%1/%2 sec.)").arg(timer3).arg(TMR_PREFILL_PUMP);
             std_text->setSubText("The pump needs to be primed for 1 minute, before being switch on");
             break;
        case S15_ADD_MALT_TO_MLT:
             string = QString("15. Add Malt to MLT (M)");
             std_text->setSubText("If malt is added, click \'Malt is Added\' on toolbar at top of screen");
             break;
//        case S16_CHILL_PUMP_FERMENTOR:
//             Std_State->Caption = "16. Chill && Pump to Fermentation Bin (M)";
//             break;
//        case S17_FINISHED:
//             Std_State->Caption = "17. Finished!";
//             break;
//        case S18_MASH_REST_5_MIN:
//             sprintf(tmp_str,"18. Mash-Rest (%d/%d sec.)",std.mrest_tmr,TMR_MASH_REST_5_MIN);
//             Std_State->Caption = tmp_str;
//             break;
//        case S19_RDY_TO_ADD_MALT:
//             Std_State->Caption = "19. Ready to add Malt to MLT (M)";
//             break;
    } // switch
    std_text->setText(string);
} // MainEbrew::SetStateName()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Edit->Edit Mash Scheme... is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuEditMashScheme(void)
{
    auto Dialog = new DialogEditMashScheme(this);

    Dialog->show();
} // MainEbrew::MenuEditMashScheme()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Edit->Fix Parameters... is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuEditFixParameters(void)
{
    auto Dialog = new DialogEditFixParameters(this);

    Dialog->show();
} // MainEbrew::MenuEditFixParameters()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Options->PID Controller Settings is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuOptionsPidSettings(void)
{
    auto Dialog = new DialogOptionsPidSettings(this);

    Dialog->show();
} // MainEbrew::MenuOptionsPidSettings()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Options->Measurements Settings is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuOptionsMeasurements(void)
{
    auto Dialog = new DialogOptionsMeasurements(this);

    Dialog->show();
} // MainEbrew::MenuOptionsMeasurements()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Options->Brew Day Settings is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuOptionsBrewDaySettings(void)
{
    auto Dialog = new DialogBrewDaySettings(this);

    Dialog->show();
} // MainEbrew::MenuOptionsBrewDaySettings()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             Options->System Settings is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuOptionsSystemSettings(void)
{
    auto Dialog = new DialogOptionsSystemSettings(this);

    Dialog->show();
} // MainEbrew::MenuOptionsSystemSettings()

/*------------------------------------------------------------------
  Purpose  : This function contains the State Transition Diagram (STD)
             for the ebrew program and is called every second.
             First: the new state is calculated based upon the conditions.
             Then: the settings of the valves are calculated and returned
             in 'kleppen'.
  Variables:
  ebrew_std: std state is updated
  Returns  : kleppen
  ------------------------------------------------------------------*/
uint16_t MainEbrew::state_machine(void)
{
    //-------------------------------------------------------------------
    //           |----------------------------> Pump 2 for HLT heat-exchanger
    //           |  |-------------------------> Main Brewing-Pump
    //           |  |  |----------------------> Future Use
    //           |  |  |  |-------------------> Out to Boil kettle
    //           |  |  |  |  |----------------> Out to Counter Flow Chiller (CFC)
    //           |  |  |  |  |  |-------------> Future Use
    //           |  |  |  |  |  |  |----------> Out to HLT Heat Exchanger
    //           |  |  |  |  |  |  |  |-------> In from Boil kettle
    //           |  |  |  |  |  |  |  |  |----> In from HLT
    //           |  |  |  |  |  |  |  |  |  |-> In from MLT
    //           P1 P0 V8 V7 V6 V5 V4 V3 V2 V1 Description                Hex.
    //-------------------------------------------------------------------------
    // State 00: 0  0  0  0  0  0  0  0  0  0  Initialisation            0x0000
    // State 01: 1  0  0  0  0  0  0  0  0  0  Wait for HLT Temp.        0x0200
    // State 02: 1  1  0  0  0  0  1  0  1  1  Fill MLT                  0x030B
    // State 03: 1  1  0  0  0  0  1  0  0  1  Wait for MLT Temp.        0x0309
    // State 04: 1  1  0  0  0  0  1  0  0  1  Mash Timer Running        0x0309
    // State 05: 1  1  0  0  0  0  1  0  0  1  Sparge Timer Running      0x0309
    // State 06: 0  1  0  1  0  0  0  0  0  1  Pump from MLT to Boil     0x0141
    // State 07: 1  1  0  0  0  0  1  0  1  1  Pump from HLT to MLT      0x030B
    // State 08: 0  0  0  0  0  0  0  0  0  0  Delay x Sec.              0x0000
    // State 09: 0  1  0  1  0  0  0  0  0  1  Empty MLT                 0x0141
    // State 10: 0  0  0  0  0  0  0  0  0  0  Wait for Boil             0x0000
    // State 11: 0  0  0  0  0  0  0  0  0  0  Boiling                   0x0000
    // State 12: 0  0  0  0  0  0  0  0  0  0  Boiling Finished          0x0000
    // State 13: 1  0  0  0  0  0  0  0  0  0  Mash PreHeat HLT          0x0200
    // State 14: 0  0  0  0  0  0  0  0  1  1  Pump Prefill              0x0003
    // State 15: 0  0  0  0  0  0  0  0  0  0  Add Malt to MLT           0x0000
    // State 16: 0  1  0  0  1  0  0  1  0  0  Chill & Pump to Fermentor 0x0124
    // State 17: 0  0  0  0  0  0  0  0  0  0  Finished!                 0x0000
    // State 18: 0  0  0  0  0  0  0  0  0  0  Mash Rest 5 minutes       0x0000
    // State 19: 1  1  0  0  0  0  1  0  0  1  Ready to add malt to MLT  0x0309
    // State 20: 0  0  0  0  0  0  0  0  0  0  CIP: Initalisation        0x0000
    // State 21: 0  1  0  1  1  0  1  1  0  0  CIP: Heat Up              0x016C
    // State 22: 0  1  0  1  1  0  1  1  0  0  CIP: Circulate 5 Minutes  0x016C
    // State 23: 0  0  0  0  0  0  0  0  0  0  CIP: Rest 5 Minutes       0x0000
    // State 24: 0  1  0  0  1  0  1  1  0  0  CIP: Drain Boil-Kettle 1  0x012C
    // State 25: 0  1  0  0  1  0  0  1  0  0  CIP: Drain Boil-Kettle 2  0x0124
    // State 26: 0  0  0  0  0  0  0  0  0  0  CIP: Fill HLT             0x0000
    // State 27: 0  1  0  1  0  0  0  0  1  0  CIP: Clean Output V7      0x0142
    // State 28: 0  1  0  0  1  0  0  0  1  0  CIP: Clean Output V6      0x0122
    // State 29: 0  1  0  0  0  0  1  0  1  0  CIP: Clean Output V4      0x010A
    // State 30: 0  0  0  0  0  0  0  1  1  0  CIP: Clean Input V3       0x0006
    // State 31: 0  0  0  0  0  0  0  0  1  1  CIP: Clean Input V1       0x0003
    // State 32: 0  0  0  0  0  0  0  0  0  0  CIP: End                  0x0000
 //----------------------------------------------------------------------------
    uint16_t  klepstanden[] = {0x0000, 0x0200, 0x030B, 0x0309, 0x0309, /* 04 */
                      /* 05 */ 0x0309, 0x0141, 0x030B, 0x0000, 0x0141, /* 09 */
                      /* 10 */ 0x0000, 0x0000, 0x0000, 0x0200, 0x0003, /* 14 */
                      /* 15 */ 0x0000, 0x0124, 0x0000, 0x0000, 0x0309, /* 19 */
                      /* 20 */ 0x0000, 0x016C, 0x016C, 0x0000, 0x012C, /* 24 */
                      /* 25 */ 0x0124, 0x0000, 0x0142, 0x0122, 0x010A, /* 29 */
                      /* 30 */ 0x0006, 0x0003, 0x0000}; /* 32 */

    uint16_t  klepstand; // Help var. = klepstanden[std->ebrew_std]

    switch (ebrew_std)
    {
        //---------------------------------------------------------------------------
        // S00_INITIALISATION: system off, wait for PID controller to be switched on
        // - If PID is enabled, then goto S01_WAIT_FOR_HLT_TEMP
        //---------------------------------------------------------------------------
        case S00_INITIALISATION:
            ms_idx    = 0;                       // init. index in mash schem
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + temp_offset0; // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-Kettle
            boil_pid->setButtonState(false);     // Disable PID-Controller button for Boil-Kettle
            if (hlt_pid->getButtonState())       // Is PowerButton pressed for HLT PID controller?
            {  // start with normal brewing states
                ebrew_std = S01_WAIT_FOR_HLT_TEMP;
            } // if
            else if (toolStartCIP->isChecked())  // Is Start CIP checkbox checked at top toolbar?
            {  // Clean-in-Place program
                ebrew_std = S20_CIP_INIT;
            } // else if
            break;

        //---------------------------------------------------------------------------
        // S01_WAIT_FOR_HLT_TEMP: Thlt < tset_HLT, continue to heat HLT
        // If Thlt > Tset_hlt AND
        //   - Malt First option is selected AND Malt has been added
        //   - OR Malt First option is NOT selected
        //  Then goto S14_PUMP_PREFILL
        //---------------------------------------------------------------------------
        case S01_WAIT_FOR_HLT_TEMP:
            toolStartCIP->setEnabled(false);     // Hide CIP option at toolbar
            toolStartAddMalt->setEnabled(true);  // Make visible
            toolMaltAdded->setEnabled(true);     // Make visible
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + temp_offset0; // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-Kettle
            if ((thlt >= tset_hlt) && (!malt_first || toolMaltAdded->isChecked()))
            {   // HLT TEMP is OK
                Vhlt_old  = Vhlt; // remember old value
                timer3    = 0;    // init. '1 minute' timer
                ebrew_std = S14_PUMP_PREFILL;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S14_PUMP_PREFILL: since the pump needs to be filled with water, prefill
        // the pump with water for a minute, then goto S02_FILL_MLT
        //---------------------------------------------------------------------------
        case S14_PUMP_PREFILL:
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + temp_offset0; // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-Kettle
            if (++timer3 >= TMR_PREFILL_PUMP)    // Stay-here timer timeout?
            {
                ebrew_std = S02_FILL_MLT;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S02_FILL_MLT: Thlt >= tset_HLT, ready to fill MLT with water from HLT
        // - Set pump on
        // - If Vmlt > dough-in volume, goto S03_MASH_IN_PROGRESS
        //---------------------------------------------------------------------------
        case S02_FILL_MLT:
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + temp_offset0; // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-Kettle
            if (Vmlt >= mash_vol)
            {
                ebrew_std = S03_WAIT_FOR_MLT_TEMP;
            } // if
            break;

        //---------------------------------------------------------------------------
        //                            M A S H I N G
        //---------------------------------------------------------------------------
        // S03_MASH_IN_PROGRESS: Start of a mash phase, Tmlt < Tset_mlt, PID is active
        // - Tset_hlt = tset (from mash scheme) + double offset or dough-in offset
        // - Start mash timer, then goto S04_MASH_TIMER_RUNNING
        //---------------------------------------------------------------------------
        case S03_WAIT_FOR_MLT_TEMP:
            // Add double offset as long as Tmlt < Tset_mlt + Offset2
            tset_mlt  = ms[ms_idx].temp; // get temp. from mash-scheme
            // tset_hlt is NOT set here, but in previous state (FILL_MLT or MASH_PREHEAT_HLT)
            tset_boil = 0.0;             // Setpoint Temp. for Boil-Kettle
            if (tmlt >= ms[ms_idx].temp + temp_offset2)
            {  // Tmlt has reached Tset_mlt + Offset2, start mash timer
                if ((ms_idx == 0) && !malt_first)
                {  // We need to add malt to the MLT first
                    // Depending on the state of the checkbox 'Leave pumps running',
                    // we go directly to state 15 or only after the user selects the
                    // 'Start adding malt to the MLT' menu-option.
                    if (leave_pumps_on)
                         ebrew_std = S19_RDY_TO_ADD_MALT; // leave pumps running for now
                    else ebrew_std = S15_ADD_MALT_TO_MLT; // switch pumps off directly
                } // if
                else
                {  // already added malt to the MLT, start directly with timer
                    ms[ms_idx].timer = 0; // start the corresponding mash timer
                    ebrew_std        = S04_MASH_TIMER_RUNNING;
                } // else
            } // if
            break;

        //---------------------------------------------------------------------------
        // S19_RDY_TO_ADD_MALT: Tmlt >= Tset_mlt, PID is active
        // - Keeps pumps running, so that water is circulating through the MLT
        // - If user presses 'OK' in dialog screen, then goto S15_ADD_MALT_TO_MLT
        //---------------------------------------------------------------------------
        case S19_RDY_TO_ADD_MALT:
            if (toolStartAddMalt->isChecked()) ebrew_std = S15_ADD_MALT_TO_MLT;
            break;

        //---------------------------------------------------------------------------
        // S15_ADD_MALT_TO_MLT: Before a mash-timer is started, the malt has to be
        // added to the MLT first. This is done only once (when ms_idx == 0). If this
        // is done, then goto S04_MASH_TIMER_RUNNING.
        //---------------------------------------------------------------------------
        case S15_ADD_MALT_TO_MLT:
            if (toolMaltAdded->isChecked())
            {  // malt is added to MLT, start mash timer
               ms[ms_idx].timer = 0; // start the corresponding mash timer
               if (mash_rest)
               {   // Start with mash rest for 5 min. after malt is added
                   mrest_tmr = 0; // init mash rest timer
                   ebrew_std = S18_MASH_REST_5_MIN;
               } // if
               else ebrew_std = S04_MASH_TIMER_RUNNING;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S04_MASH_TIMER_RUNNING: Tmlt has reached Tset_mlt (+Offset2)
        // - Tset_hlt = tset (from mash scheme) + single offset
        // - Increment mash timer until time-out
        // - If more mash phases are required, goto MASH_PREHEAT_HLT else goto SPARGE
        //---------------------------------------------------------------------------
        case S04_MASH_TIMER_RUNNING:
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + temp_offset;  // Single offset
            tset_boil = 0.0; // Setpoint Temp. for Boil-Kettle
            if (ms_idx < ms_tot - 1)
            {  // There's a next mash phase
                //if (ms[std->ms_idx].timer >= ms[std->ms_idx].time - sps->ph_timer)
                if (ms[ms_idx].timer >= ms[ms_idx].preht)
                {
                    ebrew_std = S13_MASH_PREHEAT_HLT;
                } // if
                // else remain in this state (timer is incremented)
            } // if
            else
            {  // This is the last mash phase, continue with sparging
                if (ms[ms_idx].timer >= ms[ms_idx].time) // time-out?
                {
                    sp_idx    = 0;                        // init. sparging index
                    timer1    = sp_time_ticks - 10;       // timer1 -> TIME-OUT - 10 sec.
                    ebrew_std = S05_SPARGE_TIMER_RUNNING; // goto SPARGING phase
                } // if
                // else remain in this state (timer is incremented)
            } // else
            break;

        //---------------------------------------------------------------------------
        // S13_MASH_PREHEAT_HLT: Increase Thlt PREHEAT_TIMER seconds before the time-
        //                       out of the mash timer. Heat HLT until the next temperature has
        //                       been reached.
        // - Tset_hlt = next tset (from mash scheme) + double offset
        // - Increment mash timer until time-out, then goto S03_MASH_IN_PROGRESS
        //---------------------------------------------------------------------------
        case S13_MASH_PREHEAT_HLT:
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = ms[ms_idx + 1].temp + 2 * temp_offset;
            tset_boil = 0.0; // Setpoint Temp. for Boil-Kettle
            if (ms[ms_idx].timer >= ms[ms_idx].time) // time-out?
            {
                ms_idx++; // increment index in mash scheme
                ebrew_std = S03_WAIT_FOR_MLT_TEMP;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S18_MASH_REST_5_MIN: If the 'Mash Rest' checkbox is enabled, remain for
        //                       5 minutes in this state, giving the malt time to
        //                       soak up all the water. If you leave the pump running
        //                       the filter-bed can be sucked dry.
        // - Tset_hlt = next tset (from mash scheme) + double offset
        // - Increment mash timer until time-out, then goto S03_MASH_IN_PROGRESS
        //---------------------------------------------------------------------------
        case S18_MASH_REST_5_MIN:
            ms[ms_idx].timer++; // increment mash timer
            mrest_tmr++;        // increment mash-rest timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + temp_offset; // Single offset
            tset_boil = 0.0; // Setpoint Temp. for Boil-Kettle
            if (ms_idx < ms_tot - 1)
            {  // There's a next mash phase
                if (ms[ms_idx].timer >= ms[ms_idx].preht)
                {  // Preheat timer has priority, since it also switches off the pump
                    ebrew_std = S13_MASH_PREHEAT_HLT;
                } // if
                else if (!mash_rest || (mrest_tmr >= TMR_MASH_REST_5_MIN))
                {  // after 5 min., goto normal mashing phase and switch pump on
                    ebrew_std = S04_MASH_TIMER_RUNNING;
                } // if
                // else remain in this state (timer is incremented)
            } // if
            else
            {  // Only possible when there is only one mash phase
                // This is the last mash phase, continue with sparging
                if (ms[ms_idx].timer >= ms[ms_idx].time) // time-out?
                {
                    sp_idx    = 0;                        // init. sparging index
                    timer1    = sp_time_ticks - 10;       // timer1 -> TIME-OUT - 10 sec.
                    ebrew_std = S05_SPARGE_TIMER_RUNNING; // goto SPARGING phase
                } // if
                // else remain in this state (timer is incremented)
            } // else
            break;

        //---------------------------------------------------------------------------
        //                            S P A R G I N G
        //---------------------------------------------------------------------------
        // S05_SPARGE_TIMER_RUNNING:
        // - Increment timer until time-out
        // - If more sparge phases are required, goto S06_PUMP_FROM_MLT_TO_BOIL
        //                                  else goto S09_EMPTY_MLT
        //---------------------------------------------------------------------------
        case S05_SPARGE_TIMER_RUNNING:
            tset_mlt = ms[ms_idx].temp;
            tset_hlt = tset_mlt + temp_offset; // Single offset
            if ((tboil > boil_min_temp) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-Kettle
                tset_boil = sp_preboil; // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            } // if
            else tset_boil = 0.0;
            if (++timer1 >= sp_time_ticks)
            {
                Vmlt_old  = Vmlt;  // save Vmlt for state 6 & 9
                Vboil_old = Vboil; // save Vboil for state 6 & 9
                if (sp_idx < sp_batches)
                {
                    ebrew_std = S06_PUMP_FROM_MLT_TO_BOIL; // Pump to BOIL (again)
                } // if
                else
                {  // Init flowrate low struct with percentage
                    //init_frl_struct(&frl_empty_mlt,vol->min_flowrate_mlt_perc); TODO
                    timer1    = 0;             // reset timer1
                    ebrew_std = S09_EMPTY_MLT; // Finished with Sparging, empty MLT
                } // else if
            } // if
            break;

        //---------------------------------------------------------------------------
        // S06_PUMP_FROM_MLT_TO_BOIL:
        // - Pump wort from MLT to Boil kettle until Vmlt change > sparge batch size
        // - The goto S08_DELAY_xSEC
        //---------------------------------------------------------------------------
        case S06_PUMP_FROM_MLT_TO_BOIL:
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + temp_offset; // Single offset
            if ((tboil > boil_min_temp) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-Kettle
                tset_boil = sp_preboil;         // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            } // if
            else tset_boil = 0.0;

            if (Vmlt <= Vmlt_old - (sp_idx == 0 ? sp_vol_batch0 : sp_vol_batch))
            {
                timer2    = 0; // init. x sec. timer
                ebrew_std = S08_DELAY_xSEC;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S07_PUMP_FROM_HLT_TO_MLT:
        // - Pump hot water from HLT to MLT until Vmlt change > sparge batch size
        // - The goto S05_SPARGE_TIMER_RUNNING
        //---------------------------------------------------------------------------
        case S07_PUMP_FROM_HLT_TO_MLT:
            tset_mlt = ms[ms_idx].temp;
            tset_hlt = tset_mlt + temp_offset; // Single offset
            if ((tboil > boil_min_temp) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-Kettle
                tset_boil = sp_preboil; // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            } // if
            else tset_boil = 0.0;

            if (Vmlt >= Vmlt_old + sp_vol_batch)
            {
                sp_idx++;      // Increase #Sparging Sessions done
                timer1    = 0; // init timer1
                ebrew_std = S05_SPARGE_TIMER_RUNNING;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S08_DELAY_xSEC: Delay of TIMEOUT_xSEC seconds
        //---------------------------------------------------------------------------
        case S08_DELAY_xSEC:
            tset_mlt = ms[ms_idx].temp;
            tset_hlt = tset_mlt + temp_offset; // Single offset
            if ((tboil > boil_min_temp) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-Kettle
                tset_boil = sp_preboil;         // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            } // if
            else tset_boil = 0.0;

            if (++timer2 >= TMR_DELAY_xSEC)
            {
                Vhlt_old  = Vhlt;  // remember old value
                Vmlt_old  = Vmlt;  // remember current MLT volume
                ebrew_std = S07_PUMP_FROM_HLT_TO_MLT;
            } // if
            else if (Vmlt > Vmlt_old - sp_vol_batch)
            {
                ebrew_std = S06_PUMP_FROM_MLT_TO_BOIL; // prevent false transitions
            } // else if
            break;

        //---------------------------------------------------------------------------
        //                            B O I L I N G   &    C H I L L I N G
        //---------------------------------------------------------------------------
        // S09_EMPTY_MLT: Sparging is finished, pump all wort to the boil kettle
        //---------------------------------------------------------------------------
        case S09_EMPTY_MLT:
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = 0.0;      // Disable HLT PID-Controller
            tset_boil = sp_boil;  // Boil Temperature Setpoint
            boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
//            if (flow_rate_low(vol->Flow_rate_mlt_boil,&frl_empty_mlt))
//            {
//                std->ebrew_std = S10_WAIT_FOR_BOIL;
//                std->timer5    = 0; // assure a min. stay time, so transition is detected
//            } // if
            break;

        //---------------------------------------------------------------------------
        // S10_WAIT_FOR_BOIL: After all wort is pumped to the boil kettle, it takes
        // a while before boiling actually starts. When the user selects the
        // 'Boiling Started' option or when the boil-kettle temperature exceeds
        // the minimum temperature BOIL_DETECT, goto S11_BOILING.
        //---------------------------------------------------------------------------
        case S10_WAIT_FOR_BOIL:
            tset_hlt  = 0.0;                // disable heating element
            tset_boil = sp_boil;            // Boil Temperature Setpoint
            boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            if ((++timer5 >= 10) && (/*(ui & UI_BOILING_STARTED) ||*/ (tboil > boil_detect)))
            {
                timer5    = 0; // init. timer for boiling time
                ebrew_std = S11_BOILING;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S11_BOILING: Boiling has started, remain here for BOIL_TIME minutes
        //---------------------------------------------------------------------------
        case S11_BOILING:
            tset_boil = sp_boil; // Boil Temperature Setpoint
            boil_pid->setButtonState(true); // Enable PID-Controller for Boil-Kettle
            if (++timer5 >= boil_time_ticks)
            {
                brest_tmr = 0; // init boil-rest timer
                Flow_cfc_out_reset_value = Flow_cfc_out; // reset Flow_cfc_out
                ebrew_std = S12_BOILING_FINISHED;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S12_BOILING_FINISHED: Boiling has finished, remain here until the user
        // has set up everything for chilling, then goto S15_CHILL_PUMP_FERMENTOR
        // Continu with boiling in this state, so that the brewer can manually control
        // a longer boiling time, to achieve final gravity of the wort.
        //---------------------------------------------------------------------------
        case S12_BOILING_FINISHED:
            tset_boil = 0.0;                 // Boil Temperature Setpoint
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-Kettle
            if ((!boil_rest || (++brest_tmr > TMR_BOIL_REST_5_MIN)) /*&& (ui & UI_START_CHILLING)*/)
            {  // Init flowrate low struct with percentage
                //init_frl_struct(&frl_empty_boil,vol->min_flowrate_boil_perc);
                ebrew_std = S16_CHILL_PUMP_FERMENTOR;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S16_CHILL_PUMP_FERMENTOR: The boiled wort is sent through the counterflow
        // chiller and directly into the fermentation bin.
        //---------------------------------------------------------------------------
        case S16_CHILL_PUMP_FERMENTOR:
            tset_boil = 0.0;  // Boil Temperature Setpoint
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-Kettle
            //if ((ui & UI_CHILLING_FINISHED) || (flow_rate_low(vol->Flow_rate_cfc_out,&frl_empty_boil)))
            {
                ebrew_std = S17_FINISHED;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S17_FINISHED: All wort in pumped into the fermentation bin. The brewing
        // session is finished. This is the end-state.
        //---------------------------------------------------------------------------
        case S17_FINISHED:
            tset_boil = 0.0;  // Boil Temperature Setpoint
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-Kettle
            // Remain in this state until Program Exit.
            break;

        //---------------------------------------------------------------------------
        //               C L E A N I N G    I N    P L A C E   ( C I P )
        //---------------------------------------------------------------------------
        // S20_CIP_INIT: Start of Cleaning-In-Place Program.
        // Wait until Boil-kettle has been filled with 1% NaOH solution.
        //---------------------------------------------------------------------------
        case S20_CIP_INIT:
            tset_boil = 0.0; // Boil Temperature Setpoint
            cip_circ  = 0;   // Init. CIP circulation counter
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-Kettle
//            if (ui & UI_CIP_BOIL_FILLED) // User indicated that Boil-Kettle is filled
//                ebrew_std = S21_CIP_HEAT_UP;
              /*else*/ if (!toolStartCIP->isChecked())
                  ebrew_std = S00_INITIALISATION;
            break;

        //---------------------------------------------------------------------------
        // Default: should never get here
        //---------------------------------------------------------------------------
        default:
            ebrew_std = S00_INITIALISATION;
            break;
    } // switch

    //-------------------------------------------------
    // Now calculate the proper settings for the valves
    //-------------------------------------------------
    klepstand = klepstanden[ebrew_std];
    if (!(P2->inManualMode()))
    {  // Pump 2 not in Manual Override mode
       if (klepstand & P1b) P2->setStatus(AUTO_ON);
       else                 P2->setStatus(AUTO_OFF);
    } // if
    if (!(P1->inManualMode()))
    {  // Pump 1 (Main brew pump) not in Manual Override mode
       if (klepstand & P0b) P1->setStatus(AUTO_ON);
       else                 P1->setStatus(AUTO_OFF);
    } // if
    // V8: Future Use
    if (!(V7->inManualMode()))
    {  // Valve 7 not in Manual Override mode
       if (klepstand & V7b) V7->setStatus(AUTO_ON);
       else                 V7->setStatus(AUTO_OFF);
    } // if
    if (!(V6->inManualMode()))
    {  // Valve 6 not in Manual Override mode
       if (klepstand & V6b) V6->setStatus(AUTO_ON);
       else                 V6->setStatus(AUTO_OFF);
    } // if
    // V5: Future Use
    if (!(V4->inManualMode()))
    {  // Valve 4 not in Manual Override mode
       if (klepstand & V4b) V4->setStatus(AUTO_ON);
       else                 V4->setStatus(AUTO_OFF);
    } // if
    if (!(V3->inManualMode()))
    {  // Valve 3 not in Manual Override mode
       if (klepstand & V3b) V3->setStatus(AUTO_ON);
       else                 V3->setStatus(AUTO_OFF);
    } // if
    if (!(V2->inManualMode()))
    {  // Valve 2 not in Manual Override mode
       if (klepstand & V2b) V2->setStatus(AUTO_ON);
       else                 V2->setStatus(AUTO_OFF);
    } // if
    if (!(V1->inManualMode()))
    {  // Valve 1 not in Manual Override mode
       if (klepstand & V1b) V1->setStatus(AUTO_ON);
       else                 V1->setStatus(AUTO_OFF);
    } // if
    return klepstand;
} // MainEbrew::state_machine()
