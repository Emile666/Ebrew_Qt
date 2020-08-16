/**************************************************************************************
** Filename    : MainEbrew.cpp
** Author      : Emile
** Purpose     : This file contains the MainEbrew class, which is needed for creating
**               an automated HERMS brewing system.
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
** ------------------------------------------------------------------------------------
** Revision 3.00  2020/08/12
** - Redesign from r1.99 (Borland C++ Builder) to Qt framework
**
**************************************************************************************/
#include <QApplication>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QDebug>
#include <QLabel>
#include <QElapsedTimer>
#include <QStringList>
#include <QString>
#include <QList>
#include <QCloseEvent>
#include <QThread>

#include "MainEbrew.h"
#include "dialogeditmashscheme.h"
#include "dialogeditfixparameters.h"
#include "dialogviewprogress.h"
#include "dialogviewtasklist.h"
#include "dialogoptionspidsettings.h"
#include "dialogoptionsmeasurements.h"
#include "dialogbrewdaysettings.h"
#include "dialogoptionssystemsettings.h"

//------------------------------------------------------------------------------------------
// CLASS MainEbrew
//------------------------------------------------------------------------------------------

/*------------------------------------------------------------------
  Purpose  : This is the main constructor for MainEbrew.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
MainEbrew::MainEbrew(void) : QMainWindow()
{
    SlopeLimiter *pSlopeLim; // pointer to SlopeLimiter object
    PidCtrl      *ppid;      // pointer to PidCtrl object

    createStatusBar(); // create status bar at the bottom of the screen
    createMenuBar();   // create menu bar at the top of the screen
    RegEbrew = new QSettings(REGKEY,QSettings::NativeFormat);
    if (!RegEbrew->contains("BOIL_DETECT"))
    {   // Windows registry does not exist yet
        createRegistry(); // create default Registry entries
        qDebug() << "ebrew Registry was NOT found, init. Registry with default values";
    } // if
    readMashSchemeFile(INIT_TIMERS); // Read mash scheme from file and init. all mash timers
    initBrewDaySettings();           // Init. mash, sparge and boil setting with values from Registry

    pSlopeLim = new SlopeLimiter();
    pSlopeLim->setLim(RegEbrew->value("TSET_SLOPE_LIM").toDouble());
    slopeLimHLT = pSlopeLim; // copy pointer to MainEbrew
    pSlopeLim  = new SlopeLimiter();
    pSlopeLim->setLim(RegEbrew->value("TSET_SLOPE_LIM").toDouble());
    slopeLimBK = pSlopeLim; // copy pointer to MainEbrew

    ppid = new PidCtrl(RegEbrew->value("Kc").toDouble(),RegEbrew->value("Ti").toDouble(),
                       RegEbrew->value("Td").toDouble(),RegEbrew->value("Ts").toDouble());
    PidCtrlHlt = ppid; // copy pointer to MainEbrew
    ppid = new PidCtrl(RegEbrew->value("Kc").toDouble(),RegEbrew->value("Ti").toDouble(),
                       RegEbrew->value("Td").toDouble(),RegEbrew->value("Ts").toDouble());
    PidCtrlBk = ppid; // copy pointer to MainEbrew

    //-------------------------------------------------------------
    // Wake up Ebrew hardware and try to get a decent S0 response
    //-------------------------------------------------------------
    QString srev = "SW r" + ebrewRevision.mid(11,4);
    srev.append(" HW r");
    commPortOpen();      // try to open communication channel
    int  count = 10;     // Number of retries for S0 response
    bool found = false;
    ReadDataAvailable = false;
    while (comPortIsOpen && !ReadDataAvailable && (count-- > 0) && !found)
    {
        commPortWrite("S0"); // retry
        sleep(100);          // wait until data available
        if (ReadDataAvailable)
        {
            found = (ReadData.indexOf(EBREW_HW_ID,0) != -1);
            ReadDataAvailable = false;
            qDebug() << "Found(" << found << "): " << ReadData;
        } // if
    } // while
    if (found)
    {
        srev.append(ReadData.right(4));
    } // if
    else
    {
        srev.append("?.?");
    } // else
    statusSwRev->setText(srev);

    //---------------------------------------------------
    // Open Ebrew log-file
    //---------------------------------------------------
    QFile *f = new QFile(LOGFILE); // open ebrew log-file
    if (f->open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QString d = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        fEbrewLog = f; // copy pointer to MainEbrew
        QTextStream stream(f);
        stream << "\nDate of Brewing  : " << d << "\n";
        stream << "Mash-volume      : " << mash_vol << " L, Sparge-volume: " << sp_vol << " L, boil-time: " << boil_time << " min.\n";
        stream << "Mash-scheme      : "<< statusMashScheme->text() << "\n";
        stream << "HW + SW version  : " << srev << "\n";
        stream << line1MashScheme << "\n";
        stream << "ms_tot: " << ms_tot << "\n";
        stream << "Another line\n";
        stream << " Time    TsetM TsetH  Thlt  Tmlt Telc  Vmlt s m st  GmaH  Vhlt VBoil TBoil  Tcfc GmaB\n";
        stream << "[h:m:s]   [\xB0""C]  [\xB0""C]  [\xB0""C]  [\xB0""C] [\xB0""C]   [L] p s  d   [%]   [L]   [L]  [\xB0""C]  [\xB0""C]  [%]\n";
        stream << "-------------------------------------------------------------------------------------\n";
    } // if
    else fEbrewLog = nullptr;
} // MainEbrew::MainEbrew()

/*------------------------------------------------------------------
  Purpose  : This function creates a millisecond delay. Use this
             function only during power-up and not in time-critical
             loops!
  Variables: msec: the number of milliseconds to sleep
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::sleep(uint16_t msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dieTime)
    {
        QApplication::processEvents(QEventLoop::AllEvents);
    } // while
} // MainEbrew:: sleep()

/*------------------------------------------------------------------
  Purpose  : This function is called when File->Exit is pressed.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::closeEvent(QCloseEvent *event)
{
    schedulerEbrew->stop(); // stop all tasks

    commPortWrite("B0"); // Disable Boil-kettle gas-burner
    commPortWrite("H0"); // Disable HLT gas-burner
    commPortWrite("L0"); // Disable Alive-LED
    commPortWrite("P0"); // Disable Pump
    commPortWrite("V0"); // Disable All Valves

    if (comPortIsOpen)
    {
        commPortClose(); // close the communications port
    } // if
    if (fEbrewLog)
    {
        fEbrewLog->flush(); // flush the log-file
        fEbrewLog->close(); // close the log-file
    } // if
    event->accept(); // Accept and continue with the close-event
} // MainEbrew::closeEvent()

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
    statusMashScheme = new QLabel("     ");
    statusMashScheme->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusMashScheme);
    statusMashVol    = new QLabel(" Mash Volume: 0 L ");
    statusMashVol->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusMashVol);
    statusSpargeVol  = new QLabel(" Sparge Volume: 0 L ");
    statusSpargeVol->setAlignment(Qt::AlignHCenter);
    statusBar->addPermanentWidget(statusSpargeVol);
    statusBoilTime  = new QLabel(" Boil-time: 0 min. ");
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

    // Create Toolbar1 at top of screen: always visible
    auto toolBar1 = new QToolBar("Toolbar1"); // Toolbar at left of screen
    toolBar1->setMovable(false); // fixed at top of screen
    toolBar1->setOrientation(Qt::Horizontal);
    toolStartCIP = new QCheckBox("Start Clean-In-Place (CIP)");
    toolBar1->addWidget(toolStartCIP);
    addToolBar(Qt::TopToolBarArea,toolBar1);

    // Create Toolbar2 at top of screen: brewing checkboxes
    auto toolBar2 = new QToolBar("Toolbar2"); // Toolbar at left of screen
    toolBarB = toolBar2; // save reference in MainEbrew
    toolBar2->setMovable(false); // fixed at top of screen
    toolBar2->setOrientation(Qt::Horizontal);

    toolStartAddMalt = new QCheckBox("Start Adding Malt");
    toolStartAddMalt->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolStartAddMalt);
    toolMaltAdded = new QCheckBox("Malt added to MLT");
    toolMaltAdded->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolMaltAdded);
    toolMLTEmpty = new QCheckBox("MLT is empty");
    toolMLTEmpty->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolMLTEmpty);
    toolBoilStarted = new QCheckBox("Boiling Started");
    toolBoilStarted->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolBoilStarted);
    toolStartChilling = new QCheckBox("CFC Prepared, start Chilling");
    toolStartChilling->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolStartChilling);
    toolReadyChilling = new QCheckBox("Chilling is finished");
    toolReadyChilling->setEnabled(false); // default not enabled
    toolBar2->addWidget(toolReadyChilling);
    addToolBar(Qt::TopToolBarArea,toolBar2);

    // Create Toolbar3 at top of screen: CIP checkboxes
    auto toolBar3 = new QToolBar("Toolbar3"); // Toolbar at left of screen
    toolBarC = toolBar3; // save reference in MainEbrew
    toolBar3->setMovable(false); // fixed at top of screen
    toolBar3->setOrientation(Qt::Horizontal);
    toolCipInitDone = new QCheckBox("CIP init. done");
    toolCipInitDone->setEnabled(false); // default not enabled
    toolCipDrainBK = new QCheckBox("CIP Drain Boil-kettle");
    toolCipDrainBK->setEnabled(false); // default not enabled
    toolCipHltFilled = new QCheckBox("CIP HLT Filled");
    toolCipHltFilled->setEnabled(false); // default not enabled
    toolBar3->addWidget(toolCipInitDone);
    toolBar3->addWidget(toolCipDrainBK);
    toolBar3->addWidget(toolCipHltFilled);
    addToolBar(Qt::TopToolBarArea,toolBar3);
    setTopToolBar(TOOLBAR_BREWING); // start in normal brewing mode
} // MainEbrew::createStatusBar()

/*------------------------------------------------------------------
  Purpose  : This function makes the various checkboxes visible at
             the top toolbar. There are two sets of checkboxes, one
             for normal brewing and one for clean-in-place (CIP).
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::setTopToolBar(int option)
{
    if (option == TOOLBAR_BREWING)
    {
        toolBarB->setVisible(true);
        toolBarC->setVisible(false);
    } // if
    else
    {
        toolBarB->setVisible(false);
        toolBarC->setVisible(true);
    } // else
} // MainEbrew::setTopToolBar()

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
    Fmenu->addAction(QIcon(":/img/fileopen.png"),"Read Log-File..."); // TODO slot voor Read logFile
    Fmenu->addSeparator();
    Fmenu->addAction(QIcon(":/img/exit.png")    , "E&xit", this,SLOT(close()));
    menuBar->addMenu(Fmenu);
    // Edit menu
    auto Emenu       = new QMenu("&Edit");
    Emenu->addAction(QIcon(":/img/fileedit.png") ,"&Mash Scheme..."   ,this,SLOT(MenuEditMashScheme()));
    Emenu->addAction(QIcon(":/img/fixparams.png"),"&Fix Parameters...",this,SLOT(MenuEditFixParameters()),QKeySequence("Ctrl+F"));
    menuBar->addMenu(Emenu);
    // View menu
    auto Vmenu       = new QMenu("&View");
    Vmenu->addAction(QIcon(":/img/progress.png"),"&Mash && Sparge Progress",this,SLOT(MenuViewProgress()));
    Vmenu->addAction(QIcon(":/img/alarm.png")   ,"&Status and Alarms");          // TODO slot voor Status and Alarms
    Vmenu->addAction(QIcon(":/img/task.png")    ,"&Task-list and Timings"  ,this,SLOT(MenuViewTaskList()));
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
    RegEbrew->setValue("GAS_NON_MOD_LLIMIT",30); // Parameter 1 for Ebrew HW
    RegEbrew->setValue("GAS_NON_MOD_HLIMIT",35); // Parameter 2 for Ebrew HW
    RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",2);  // Parameter 3 for Ebrew HW
    RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",4);  // Parameter 4 for Ebrew HW
    RegEbrew->setValue("TTRIAC_LLIM",65);        // Parameter 5 for Ebrew HW
    RegEbrew->setValue("TTRIAC_HLIM",75);        // Parameter 6 for Ebrew HW
    // Communications
    RegEbrew->setValue("SYSTEM_MODE",GAS_MODULATING);         // Parameter 0 for Ebrew HW
    RegEbrew->setValue("COMM_CHANNEL",0);                     // Select Ethernet as Comm. Channel
    RegEbrew->setValue("COM_PORT_SETTINGS","38400,N,8,1");    // COM port settings
    RegEbrew->setValue("UDP_IP_PORT","192.168.192.105:8888"); // IP & Port number TODO: use it (UDP_IP_PORT)
    RegEbrew->setValue("CB_DEBUG_COM_PORT",1);                // 1 = log COM port read/writes
    // Brew-kettle Sizes
    RegEbrew->setValue("VHLT_MAX",200);       // Max. HLT volume
    RegEbrew->setValue("VMLT_MAX",110);       // Max. MLT volume
    RegEbrew->setValue("VBOIL_MAX",140);      // Max. Boil kettle volume

    //------------------------------------
    // Options -> PID Settings Dialog
    //------------------------------------
    RegEbrew->setValue("TS",20.0);            // Set Default sample time [sec.]
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
    RegEbrew->setValue("SP_MPY",2.0);        // Multiply factor for 1st run-off volume to Boil-kettle
    // Mash Settings
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
    RegEbrew->setValue("BOIL_MIN_TEMP",60);  // Min. Temp. for Boil-kettle (Celsius)
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
    RegEbrew->setValue("MIN_FR_BOIL_PERC",2); // Min. Flowrate for Boil-kettle Empty detection
} // MainEbrew::createRegistry()

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
  Purpose  : This function reads a Mash Scheme from a .sch file and
             updates the statusbar with the new temp-time pairs.
             This function is called after the Registry has been
             initialized, so it is safe to use Registry values.
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
       if (!in.atEnd()) line1MashScheme = in.readLine();
       line1MashScheme.remove(0,3); // remove the '// ' characters
       while ((i++ < 2) && !in.atEnd())
       {  // read 2 dummy lines
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
                 ms[ms_tot].time_stamp.clear();          /* init. time-stamp to empty string */
              } // if
              if (!sbar.isEmpty()) sbar.append(", ");
              sbar.append(QString("%1 °C(%2 min.)").arg(ms[ms_tot].temp,2,'f',0).arg(ms[ms_tot].time/60.0,2,'f',0));
              ms_tot++; // number of total temperature-time pairs
          } // if
       } // while
       statusMashScheme->setText(sbar);
       inputFile.close();
    }  // if
    else QMessageBox::warning(this,"File-read error","Could not open " MASHFILE);
} // MainEbrew::readMashSchemeFile()

/*------------------------------------------------------------------
  Purpose  : This function initializes the mash, sparge and boil
             settings with the value from Registry. Call this method
             during power-up or after a change in Brew-day Settings.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::initBrewDaySettings(void)
{
    sp_vol_batch  = sp_vol / RegEbrew->value("SP_BATCHES").toDouble();
    sp_vol_batch0 = sp_vol_batch * RegEbrew->value("SP_MPY").toDouble();
    ph_time       = 60 * RegEbrew->value("PREHEAT_TIME").toInt(); // ph_time is in seconds
    for (int i = 0; i < ms_tot; i++)
    {
        ms[i].preht = ms[i].time;
        if (RegEbrew->value("CB_dpht").toInt())
        {   // dynamic preheat time
            if (i < ms_tot - 1)
                 ms[i].preht -= RegEbrew->value("HLT_Bcap").toInt() * (ms[i+1].temp - ms[i].temp);
            else ms[i].preht = 0;
        } // if
        else
        {   // fixed preheat time
            ms[i].preht -= ph_time;
        } // else
        if (ms[i].preht < 0) ms[i].preht = 0;
    } // for i
    //--------------------------------------------------------------
    // Now calculate the internal values for the timers. Since the
    // State Transition Diagram is called every second, every 'tick'
    // is a second.
    // From Registry        : SP_TIME   [min.]
    // From mash-scheme file: boil_time [min.]
    //--------------------------------------------------------------
    sp_time_ticks   = 60 * RegEbrew->value("SP_TIME").toInt();
    boil_time_ticks = 60 * boil_time;
} // MainEbrew::initBrewDaySettings()

/*------------------------------------------------------------------
  Purpose  : This function handles the key-press events
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case     Qt::Key_1: V1->setNextStatus(); break;
        case     Qt::Key_2: V2->setNextStatus(); break;
        case     Qt::Key_3: V3->setNextStatus(); break;
        case     Qt::Key_4: V4->setNextStatus(); break;
        case     Qt::Key_6: V6->setNextStatus(); break;
        case     Qt::Key_7: V7->setNextStatus(); break;
        case     Qt::Key_A: V1->setStatus(AUTO_OFF);
                            V2->setStatus(AUTO_OFF);
                            V3->setStatus(AUTO_OFF);
                            V4->setStatus(AUTO_OFF);
                            V6->setStatus(AUTO_OFF);
                            V7->setStatus(AUTO_OFF);
                            P1->setStatus(AUTO_OFF);
                            P2->setStatus(AUTO_OFF);
                            break;
        case     Qt::Key_P: P1->setNextStatus(); // main pump
                            break;
        case     Qt::Key_Q: P2->setNextStatus(); // pump for HLT heat-exchanger
                            break;
        case     Qt::Key_R: commPortWrite("R0"); // reset all flows to 0.0 L in ebrew hardware
                            break;
        case     Qt::Key_S: // toggle between various alarms or no alarm
                            if (++alarmSound > ALARM_TEMP_FLOW_SENSORS) alarmSound = ALARM_OFF;
                            switch (alarmSound)
                            {
                                 case ALARM_OFF:               statusAlarm->setText(" Sensor Alarm: OFF ")      ; break;
                                 case ALARM_TEMP_SENSORS:      statusAlarm->setText(" Sensor Alarm: TEMP ")     ; break;
                                 case ALARM_FLOW_SENSORS:      statusAlarm->setText(" Sensor Alarm: FLOW ")     ; break;
                                 case ALARM_TEMP_FLOW_SENSORS: statusAlarm->setText(" Sensor Alarm: TEMP+FLOW "); break;
                            } // switch
                            break;
        default:            break;
    } // switch
} // MainEbrew::keyPressEvent()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: ALIVE Led toggle
  Period-Time: 0.5 second
  ---------------------------------------------------------------------------*/
void MainEbrew::task_alive_led(void)
{
    QElapsedTimer timer;

    timer.start();

    hlt->setValues(thlt,tset_hlt,Vhlt,gamma_hlt); // temp, sp, vol, power
    hlt->update();

    mlt->setValues(tmlt,0.0,Vmlt,0.0);
    mlt->update();

    boil->setValues(tboil,tset_boil,Vboil,gamma_boil); // temp, sp, vol, power
    boil->update();

    T3->setTempValue(tcfc); // Separate temp. sensor for Tcfc
    T3->update();

    //------------------------------------------------------------------
    // Toggle alive toggle bit to see if this routine is still alive
    //------------------------------------------------------------------
    toggleLed = !toggleLed;
    QString string = QString("L%1").arg(toggleLed ? 1 : 0); // send Alive Led to ebrew hardware
    commPortWrite(string.toUtf8());
    schedulerEbrew->updateDuration("aliveLed",timer.nsecsElapsed()/1000);
} // MainEbrew::task_alive_led()

/*------------------------------------------------------------------
  Purpose    : TASK: Ebrew State Transition Diagram (STD).
  Period-Time: 1.0 second
  ------------------------------------------------------------------*/
void MainEbrew::task_update_std(void)
{
    QElapsedTimer timer;
    QString       string;
    QColor        color;
    uint16_t      std_out;     // values of valves
    uint8_t       pump_bits;   // values of pumps

    timer.start(); // Task time-measurement
    std_out = state_machine(); // call the Ebrew STD
    if (tset_hlt_sw)
    {  // Set Temperature Setpoint for HLT to a fixed value
       tset_hlt = tset_hlt_fx; // fix tset_hlt
    } // if
    tset_hlt = slopeLimHLT->slopeLimit(tset_hlt);
    if (tset_boil_sw)
    {  // Set Temperature Setpoint for Boil-kettle to a fixed value
       tset_boil = tset_boil_fx;
    } // if
    tset_boil = slopeLimBK->slopeLimit(tset_boil);

    //---------------------------------------------------------------------
    // Output all valve bits to the Ebrew hardware with the Vx command.
    //---------------------------------------------------------------------
    string = QString("V%1").arg(std_out & ALL_VALVES); // Output all valves
    commPortWrite(string.toUtf8());

    //---------------------------------------------------------------------
    // Send Pump On/Off signals to ebrew hardware with the Px command.
    //---------------------------------------------------------------------
    if (std_out & P0b) pump_bits  = 0x01;
    else               pump_bits  = 0x00;
    if (std_out & P1b) pump_bits |= 0x02;
    string = QString("P%1").arg(pump_bits); // Output all pumps
    commPortWrite(string.toUtf8());

    //---------------------------------------------------------------------
    // Calculate if there's any flow running and if so, set the proper
    // colours for the pipes.
    //---------------------------------------------------------------------
    bool anyInputOn  = (std_out & (V1b | V2b | V3b)) > 0x0000;
    bool anyOutputOn = (std_out & (V4b | V6b | V7b)) > 0x0000;
    bool pumpP1On    = (std_out & (P0b)) > 0x0000;
    bool pumpP2On    = (std_out & (P1b)) > 0x0000;

    //-------------------------------------------------
    // Is there output-flow from Pump P1?
    //-------------------------------------------------
    if (pumpP1On && anyInputOn && anyOutputOn)
         color = COLOR_OUT1; // flow running
    else color = COLOR_OUT0; // no flow
    elbow6->setColor(color);
    Tpipe3->setColor(color);

    //-------------------------------------------------
    // Output pump -> HLT heat-exchanger -> MLT return
    //-------------------------------------------------
    if (pumpP1On && anyInputOn && (std_out & V4b))
    {
         color        = COLOR_OUT1; // flow running
         flow4Running = true;       // flow4 should see a flowrate
    } // if
    else
    {
        color        = COLOR_OUT0; // no flow
        flow4Running = false;      // flow4 is not running
    } // else
    pipeH5->setColor(color);
    elbow10->setColor(color);
    pipeV3->setColor(color);
    pipeV2->setColor(color);
    elbow11->setColor(color);
    pipeH9->setColor(color);
    elbow8->setColor(color);
    elbow9->setColor(color);
    hlt->setColor(COLOR_RIGHT_PIPES,color);
    mlt->setColor(COLOR_TOP_PIPE,color);

    //-------------------------------------------------
    // Is there output-flow to either V6 or V7?
    //-------------------------------------------------
    if (pumpP1On && anyInputOn && (std_out & (V6b | V7b)))
         color = COLOR_OUT1; // flow running
    else color = COLOR_OUT0; // no flow
    pipeH6->setColor(color);
    Tpipe4->setColor(color);

    //-------------------------------------------------
    // Is there output-flow to V6?
    //-------------------------------------------------
    if (pumpP1On && anyInputOn && (std_out & V6b))
    {
        color        = COLOR_OUT1; // flow running
        flow3Running = true;       // flow3 (CFC-output) should see a flowrate
    } // if
    else
    {
        color        = COLOR_OUT0; // no flow
        flow3Running = false;      // flow3 is not running
    } // else
    pipeV4->setColor(color);
    elbow5->setColor(color);
    pipeH7->setColor(color);

    //-------------------------------------------------
    // Is there output-flow to V7?
    //-------------------------------------------------
    if (pumpP1On && anyInputOn && (std_out & V7b))
    {
        color = COLOR_OUT1;  // flow running
        flow2Running = true; // flow2 (BK-input) should see a flowrate
    } // if
    else
    {
        color = COLOR_OUT0;   // no flow
        flow2Running = false; // flow2 is not running
    } // else
    pipeH8->setColor(color);
    elbow4->setColor(color);
    pipeV5->setColor(color);
    boil->setColor(COLOR_BOTTOM_PIPE2,color);

    //-------------------------------------------------
    // Is there input-flow to Pump P1?
    //-------------------------------------------------
    if (pumpP1On && anyOutputOn && (std_out & (V1b | V2b | V3b)))
         color = COLOR_IN1; // flow running
    else color = COLOR_IN0; // no flow
    pipeH4->setColor(color);
    elbow7->setColor(color);
    pipeV1->setColor(color);
    Tpipe1->setColor(color);

    //-------------------------------------------------
    // Is there input-flow from Valve 2 (HLT)?
    //-------------------------------------------------
    if (pumpP1On && anyOutputOn && (std_out & V2b))
    {
        color        = COLOR_IN1; // flow running
        flow1Running = true;      // FLOW1 HLT->MLT should see a flowrate
    } // if
    else
    {
        color        = COLOR_IN0; // no flow
        flow1Running = false;     // FLOW1 HLT->MLT is not running
    } // else
    elbow2->setColor(color);
    hlt->setColor(COLOR_BOTTOM_PIPE1,color);

    //---------------------------------------------------------------
    // Is there input-flow from either Valve 1 (MLT) or Valve 3 (BK)?
    //---------------------------------------------------------------
    if (pumpP1On && anyOutputOn && (std_out & (V1b | V3b)))
         color = COLOR_IN1; // flow running
    else color = COLOR_IN0; // no flow
    pipeH2->setColor(color);
    Tpipe2->setColor(color);

    //-------------------------------------------------
    // Is there input-flow from Valve 1 (MLT)?
    //-------------------------------------------------
    if (pumpP1On && anyOutputOn && (std_out & V1b))
         color = COLOR_IN1; // flow running
    else color = COLOR_IN0; // no flow
    mlt->setColor(COLOR_BOTTOM_PIPE1,color);

    //-------------------------------------------------
    // Is there input-flow from Valve 3 (BK)?
    //-------------------------------------------------
    if (pumpP1On && anyOutputOn && (std_out & V3b))
         color = COLOR_IN1; // flow running
    else color = COLOR_IN0; // no flow
    pipeH3->setColor(color);
    elbow3->setColor(color);
    boil->setColor(COLOR_BOTTOM_PIPE1,color);

    //-------------------------------------------------
    // Is the pump for the HLT heat-exchanger on?
    //-------------------------------------------------
    if (pumpP2On)
         color = COLOR_IN1;
    else color = COLOR_IN0;
    elbowP20->setColor(color);
    elbowP21->setColor(color);
    elbowP22->setColor(color);
    elbowP23->setColor(color);
    pipeH1->setColor(color);
    hlt->setColor(COLOR_LEFT_PIPES,color);

    schedulerEbrew->updateDuration("updateStd",timer.nsecsElapsed()/1000);
} // MainEbrew::task_update_std()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: PID-controller
  Period-Time: TS seconds
  ---------------------------------------------------------------------------*/
void MainEbrew::task_pid_control(void)
{
    QElapsedTimer timer;
    QString       string;

    timer.start(); // Task time-measurement

    PidCtrlHlt->pidEnable(hlt_pid->getButtonState());     // PID is enabled if PowerButton state is ON
    gamma_hlt = PidCtrlHlt->pidControl(thlt,tset_hlt);    // run pid-controller for HLT
    if (gamma_hlt_sw) gamma_hlt = gamma_hlt_fx;
    string = QString("H%1").arg(gamma_hlt,1,'f',0);       // PID-Output [0%..100%]
    commPortWrite(string.toUtf8());

    if (PidCtrlBk->pidGetStatus() != PID_FFC)             // PID_FFC is set by the STD
        PidCtrlBk->pidEnable(boil_pid->getButtonState()); // PID is enabled if PowerButton state is ON
    gamma_boil = PidCtrlBk->pidControl(tboil,tset_boil);  // run pid-controller for Boil-kettle
    if ((ebrew_std == S11_BOILING) && (gamma_boil > RegEbrew->value("LIMIT_BOIL").toDouble()))
    {   // Limit Boil-kettle output during boiling
        gamma_boil = RegEbrew->value("LIMIT_BOIL").toDouble();
    } // if
    if (gamma_boil_sw) gamma_boil = gamma_boil_fx;

    string = QString("B%1").arg(gamma_boil,1,'f',0);      // PID-Output [0%..100%]
    commPortWrite(string.toUtf8());
    schedulerEbrew->updateDuration("pidControl",timer.nsecsElapsed()/1000);
} // MainEbrew::task_pid_control()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: Read all temperature values from Ebrew hardware
  Period-Time: 2 seconds
  ---------------------------------------------------------------------------*/
void MainEbrew::task_read_temps(void)
{
    QElapsedTimer  timer;
    int            count = 0;

    timer.start();
    commPortWrite("A0"); // A0 = Read all temperature values from Ebrew hardware
    while (comPortIsOpen && !ReadDataAvailable && (count++ < MAX_READ_RETRIES))
    {
        sleep(NORMAL_READ_TIMEOUT);
    } // while
    // Check string received for header and length "T=0.00,0.00,0.00,0.00,0.00"
    if (ReadDataAvailable && (ReadData.indexOf("T=") != -1) && (ReadData.size() >= 26))
    {
        QByteArrayList list = ReadData.split(','); // split array in sub-arrays
        if (list.size() >= 5)
        {   // at least 5 temperature values
            QByteArray ba = list.at(0);
            ba.remove(0,2); // remove "T=" in 1st byte-array
            ttriac  = ba.toDouble();
            thlt    = list.at(1).toDouble();
            tmlt    = list.at(2).toDouble();
            tboil   = list.at(3).toDouble();
            tcfc    = list.at(4).toDouble();
        } // if
    } // if
    else qDebug() << "task_read_temps() error: " << ReadData; // error

    //------------------ TEMP1 (LM35) -----------------------------------------
    if (ttriac_sw)
    {  // Switch & Fix
       ttriac = ttriac_fx;
    } // if
    //---------------------------------------------------
    // Triac Temperature Protection: hysteresis function
    //---------------------------------------------------
    if (triacTooHot)
    { // Reset if temp. < lower-limit
      triacTooHot = (ttriac >= RegEbrew->value("TTRIAC_LLIM").toInt());
    } // if
    else
    { // set if temp. >= upper-limit
      triacTooHot = (ttriac >= RegEbrew->value("TTRIAC_HLIM").toInt());
    } // else
    //------------------ TEMP2 (THLT) -----------------------------------------
    if (thlt > SENSOR_VAL_LIM_OK)
    {
         thlt += RegEbrew->value("THLT_OFFSET").toDouble(); // update THLT with calibration value
         sensorAlarmInfo &= ~SENS_THLT;      // reset bit in sensor_alarm
    } // if
    else
    {
         if ((alarmSound == ALARM_TEMP_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
              commPortWrite("X3");
              sensorAlarmInfo |= SENS_THLT;
         } // if
    } // else
    if (thlt_sw)
    {  // Switch & Fix
       thlt = thlt_fx;
    } // if
    //------------------ TEMP3 (TMLT) -----------------------------------------
    if (tmlt > SENSOR_VAL_LIM_OK)
    {
         tmlt += RegEbrew->value("TMLT_OFFSET").toDouble(); // update TMLT with calibration value
         sensorAlarmInfo &= ~SENS_TMLT; // reset bit in sensor_alarm
    } // if
    else
    {
        if ((alarmSound == ALARM_TEMP_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
        {
             commPortWrite("X3\n");
             sensorAlarmInfo |= SENS_TMLT;
        } // if
    } // else
    if (tmlt_sw)
    {  // Switch & Fix
       tmlt = tmlt_fx;
    } // if
    //------------------ TEMP4 (TBOIL) ----------------------------------------
    if (tboil > SENSOR_VAL_LIM_OK)
    {
         tboil += RegEbrew->value("TBOIL_OFFSET").toDouble(); // update TBOIL with calibration value
         sensorAlarmInfo &= ~SENS_TBOIL; // reset bit in sensor_alarm
    } // if
    else
    {
         if ((alarmSound == ALARM_TEMP_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
              commPortWrite("X3\n");
              sensorAlarmInfo |= SENS_TBOIL;
         } // if
    } // else
    if (tboil_sw)
    {  // Switch & Fix
       tboil = tboil_fx;
    } // if
    //------------------ TEMP5 (TCFC) -----------------------------------------
    if (tcfc > SENSOR_VAL_LIM_OK)
    {
         tcfc += RegEbrew->value("TCFC_OFFSET").toDouble(); // update TCFC with calibration value
         sensorAlarmInfo &= ~SENS_TCFC;      // reset bit in sensor_alarm
    } // if
    else
    {
         if ((alarmSound == ALARM_TEMP_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
              commPortWrite("X3\n");
              sensorAlarmInfo |= SENS_TCFC;
         } // if
    } // else
    // No switch/fix needed for TCFC
    schedulerEbrew->updateDuration("readTemps",timer.nsecsElapsed()/1000);
} // MainEbrew::task_read_temps()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: Read all flow sensors from Ebrew hardware
  Period-Time: 2 seconds
  ---------------------------------------------------------------------------*/
void MainEbrew::task_read_flows(void)
{
    QElapsedTimer  timer;
    int            count = 0;

    timer.start();
    commPortWrite("A9"); // Read all flowsensor values from Ebrew hardware
    while (comPortIsOpen && !ReadDataAvailable && (count++ < MAX_READ_RETRIES))
    {   // give E-brew hardware time to react
        sleep(NORMAL_READ_TIMEOUT);
    } // while
    // Check string received for header and length "F=0.00,0.00,0.00,0.00"
    if (ReadDataAvailable && (ReadData.indexOf("F=") != -1) && (ReadData.size() >= 21))
    {
        QByteArrayList list = ReadData.split(','); // split array in sub-arrays
        if (list.size() >= 4)
        {   // at least 4 flowsensors
            QByteArray ba = list.at(0);
            ba.remove(0,2); // remove "F=" in 1st byte-array
            FlowHltMlt  = ba.toDouble();
            FlowMltBoil = list.at(1).toDouble();
            FlowCfcOut  = list.at(2).toDouble();
            Flow4       = list.at(3).toDouble();
        } // if
    } // if
    else
    {   // error
        qDebug() << "task_read_flows() error: " << ReadData;
    } // else

    F1->setFlowValue(FlowHltMlt ,thlt);
    F2->setFlowValue(FlowMltBoil,tmlt);
    F3->setFlowValue(FlowCfcOut ,tcfc);
    F4->setFlowValue(Flow4      ,thlt);

    //------------------ FLOW1 ------------------------------------------------
    if ((F1->getFlowRate(FLOWRATE_RAW) < 0.1) && flow1Running &&
        ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS)))
    {
         commPortWrite("X2\n");
         sensorAlarmInfo |=  SENS_FLOW1;
         F1->setError(true);
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW1;
        F1->setError(false);
    } // else
    Vhlt = RegEbrew->value("VHLT_MAX").toDouble()- FlowHltMlt;
    if (vhlt_sw)
    {  // Switch & Fix
       Vhlt = vhlt_fx;
    } // if

    //------------------ FLOW2 ------------------------------------------------
    if ((F2->getFlowRate(FLOWRATE_RAW) < 0.1) && flow2Running &&
        ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS)))
    {
         commPortWrite("X2\n"); // sound alarm
         sensorAlarmInfo |=  SENS_FLOW2;
         F2->setError(true);
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW2;
        F2->setError(false);
    } // else
    Vmlt = FlowHltMlt - FlowMltBoil;
    if (vmlt_sw)
    {  // Switch & Fix
       Vmlt = vmlt_fx;
    } // if

    //------------------ FLOW3 ------------------------------------------------
    if ((F3->getFlowRate(FLOWRATE_RAW) < 0.1) && flow3Running &&
        ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS)))
    {
         commPortWrite("X2\n"); // sound alarm
         sensorAlarmInfo |=  SENS_FLOW3;
         F3->setError(true);
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW3;
        F3->setError(false);
    } // else
    Vboil = FlowMltBoil - FlowCfcOut;
    if (vboil_sw)
    {  // Switch & Fix
       Vboil = vboil_fx;
    } // if

    //------------------ FLOW4 ------------------------------------------------
    if ((F4->getFlowRate(FLOWRATE_RAW) < 0.1) && flow4Running &&
        ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS)))
    {
         commPortWrite("X2\n"); // sound alarm
         sensorAlarmInfo |=  SENS_FLOW4;
         F4->setError(true);
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW4;
        F4->setError(false);
    } // else

    schedulerEbrew->updateDuration("readFlows",timer.nsecsElapsed()/1000);
} // task_read_flows()

/*-----------------------------------------------------------------------------
  Purpose    : TASK: Write all relevant data to a log-file.
  Period-Time: 5 seconds
  ---------------------------------------------------------------------------*/
void MainEbrew::task_write_logfile()
{
    QString        string;
    QElapsedTimer  timer;

    timer.start(); // Start time-measurement
    if (fEbrewLog != nullptr)
    {   // Ebrew log-file is opened
        QTextStream stream(fEbrewLog);
        QTime       d = QTime::currentTime();

        string = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15")
                         .arg(tset_mlt,5,'f',2)    /* Setpoint temperature for MLT */
                         .arg(tset_hlt,5,'f',2)    /* Setpoint temperature for HLT */
                         .arg(thlt,5,'f',2)        /* HLT actual temperature */
                         .arg(tmlt,5,'f',2)        /* MLT actual temperature */
                         .arg(ttriac,4,'f',1)      /* Temperature of power-electronics */
                         .arg(Vmlt,5,'f',1)        /* MLT actual volume */
                         .arg(sp_idx)              /* current sparge-index */
                         .arg(ms_idx)              /* current mash-index */
                         .arg(ebrew_std,2)         /* Current state of STD */
                         .arg(gamma_hlt,5,'f',1)   /* PID output for HLT */
                         .arg(Vhlt,5,'f',1)        /* HLT actual volume */
                         .arg(Vboil,5,'f',1)       /* Boil-kettle actual volume */
                         .arg(tboil,5,'f',1)       /* Boil-kettle actual temperature */
                         .arg(tcfc,4,'f',1)        /* CFC actual temperature */
                         .arg(gamma_boil,5,'f',1); /* PID output for Boil-kettle */
        stream << d.toString("hh:mm:ss") << "," << string << "\n";
    } // if
    schedulerEbrew->updateDuration("wrLogFile",timer.nsecsElapsed()/1000);
} // MainEbrew::task_write_logfile()

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
                "This version (Ebrew 3.0 Qt) is a complete redesign and is built with Qt 5.14<br><br>"
                "Web-site: <a href=\"www.vandelogt.nl\">www.vandelogt.nl</a><br>"
                "Brewery: Brouwerij de Boezem, The Netherlands");
} // MainEbrew::about()

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
             View->Mash & Sparge Progress is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuViewProgress(void)
{
    auto Dialog = new DialogViewProgress(this);

    Dialog->show();
} // MainEbrew::MenuViewProgress()

/*------------------------------------------------------------------
  Purpose  : This function is called whenever in the Menubar
             View->Task-list and Timing is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuViewTaskList(void)
{
    auto Dialog = new DialogViewTaskList(this);

    Dialog->show();
} // MainEbrew::MenuViewTaskList()

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
  Purpose  : This function displays a messagebox and makes sure only
             one instance is created.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::msgBox(QString title, QString text, QCheckBox *cb)
{
    static bool state = false;

    if (!state)
    {
        state = true;
        int retv = QMessageBox::warning(this, title, text, QMessageBox::Ok);
        if (retv == QMessageBox::Ok)
        {
            state = false;
            cb->setChecked(true);
        } // if
    } // if
} // MainEbrew::msgBox()

/*------------------------------------------------------------------
  Purpose  : This function opens a communication channel. This can be
             either an Ethernet UDP or a virtual COM port over USB.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::commPortOpen(void)
{
    int x = RegEbrew->value("COMM_CHANNEL").toInt();
    QSerialPort::Parity y;
    QString      string = RegEbrew->value("COM_PORT_SETTINGS").toString();
    QStringList  list   = string.split(',');
    QSerialPort *p      = new QSerialPort;

    serialPort = p; // copy pointer to MainEbrew
    comPortIsOpen = false;

    if (x > 0)
    {   // Any of the Virtual USB COM ports
        p->setPortName(QString("COM%1").arg(x));
        if (p->open(QIODevice::ReadWrite))
        {   //Now the serial port is open. Try to set configuration
            comPortIsOpen = true;
            if (list.size() != 4)
               qDebug() << "COM_PORT_SETTINGS not set correctly: " << string;
            else
            {
                if (!p->setBaudRate(list.at(0).toInt()))
                    qDebug() << p->errorString();
                switch (toupper(list.at(1).toInt()))
                {
                    case 'N': y = QSerialPort::NoParity  ; break;
                    case 'O': y = QSerialPort::EvenParity; break;
                    case 'E': y = QSerialPort::OddParity ; break;
                    default : y = QSerialPort::NoParity  ; break;
                } // switch
                if(!p->setParity(y))
                    qDebug() << p->errorString();
                if(!p->setDataBits((QSerialPort::DataBits)list.at(2).toInt()))
                    qDebug() << p->errorString();
                if(!p->setStopBits((QSerialPort::StopBits)list.at(3).toInt()))
                    qDebug() << p->errorString();
                if(!p->setFlowControl(QSerialPort::NoFlowControl))
                    qDebug() << p->errorString();
                connect(p, &QSerialPort::readyRead,this,&MainEbrew::commPortRead);
            } // else
        } // if
        else qDebug()<<"Serial port"<< p->portName() << " not opened. Error: " << p->errorString();
    } // if
    else
    {   // Ethernet UDP connection

    } // else
    if (RegEbrew->value("CB_DEBUG_COM_PORT").toInt())
    {
        QFile *f = new QFile(COMMDBGFILE); // open comm debug log file
        if (f->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            QString d = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
            fDbgCom = f; // copy pointer to MainEbrew
            QTextStream stream(f);
            stream << "----------------------------------------------------------------------\n" <<
                      "File opened: " << d << " ";
            if (x > 0)
            {
                stream << p->portName() << " ";
                if (comPortIsOpen) stream << "open for read/write.";
                else               stream << "NOT opened.";
            } // if
            else       stream << "Ethernet: " << RegEbrew->value("UDP_IP_PORT").toByteArray();
            stream << "\n";
        } // if
        else fDbgCom = nullptr;
    } // if
    else fDbgCom = nullptr;
} // MainEbrew::commPortOpen()

/*------------------------------------------------------------------
  Purpose  : This function closes the communications channel. This can
             be an Ethernet UDP or a virtual COM port over USB.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::commPortClose(void)
{
    if (RegEbrew->value("COMM_CHANNEL").toInt() > 0)
    {   // Any of the Virtual USB COM ports
        if (comPortIsOpen)
        {
            serialPort->flush(); // send data in buffers to serial port
            serialPort->close(); // close the serial port
            comPortIsOpen = false;
            ReadDataAvailable = false;
            ReadData.clear();
        } // if
    } // if
    if (fDbgCom != nullptr)
    {
        QString d = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        QTextStream stream(fDbgCom);
        stream << "\nFile closed: " << d << "\n" <<
                  "----------------------------------------------------------------------\n\n";
        fDbgCom->flush();
        fDbgCom->close();
    } // if
} // MainEbrew::commPortClose()

/*------------------------------------------------------------------
  Purpose  : This function writes a string to the communications channel.
             This can be an Ethernet UDP or a virtual COM port over USB.
  Variables: s: the string to write to the communications channel.
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::commPortWrite(QByteArray s)
{
    if (RegEbrew->value("COMM_CHANNEL").toInt() > 0)
    {   // Any of the Virtual USB COM ports
        if (comPortIsOpen)
        {
            ReadDataAvailable = false;
            ReadData.clear();
            serialPort->write(s + "\n"); // add newline character
            if (!serialPort->waitForBytesWritten(100))
                qDebug() << "CommPortWrite() timeout";
        } // if
        else qDebug() << "CommPortWrite(): COM port not open";
    } // if
    else
    {   // Ethernet UDP connection

    } // else
    if (fDbgCom != nullptr)
    {   // comm. debug-logging is enabled
        QTime d = QTime::currentTime();
        QTextStream stream(fDbgCom);
        stream << "\nW" << d.toString("ss") << "." << d.toString("zzz") << "[" << s << "]";
    } // if
} // MainEbrew::commPortWrite()

/*------------------------------------------------------------------
  Purpose  : This function reads a string from the communications channel.
             This can be an Ethernet UDP or a virtual COM port over USB.
             For the Virtual COM port, this routine is used in a
             signal-slot combination. Signal is readyRead() from the
             SerialPort, slot is this function.
  Variables: ReadData         : the data read from the Virtual COM port
             ReadDataAvailable: true = new data is available
  Returns  : the string read from the communications channel.
  ------------------------------------------------------------------*/
void MainEbrew::commPortRead(void)
{
    if (RegEbrew->value("COMM_CHANNEL").toInt() > 0)
    {   // Any of the Virtual USB COM ports
        ReadData.append(serialPort->readAll());
        removeLF(ReadData); // remove \n
        ReadDataAvailable = true;
    } // if
    else
    {   // Ethernet UDP connection

    } // else

    if (fDbgCom != nullptr)
    {   // comm. debug-logging is enabled
        QTime d = QTime::currentTime();
        QTextStream stream(fDbgCom);
        stream << " R" << d.toString("ss") << "." << d.toString("zzz") << "[" << ReadData << "]";
        if (serialPort->error() == QSerialPort::ReadError) stream << " Read-error: " << serialPort->errorString();
    } // if
} // MainEbrew::commPortRead()

/*------------------------------------------------------------------
  Purpose  : This function removes all newline (\n) chars from a string.
  Variables: s: the string with the \n in it.
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::removeLF(QByteArray &s)
{
    int i;
    while (!s.isEmpty() && ((i = s.indexOf('\n',0)) > -1))
    {
        s.remove(i,1); // remove \n
    } // while
} // MainEbrew::removeLF()

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
    // State 24: 0  1  0  0  1  0  1  1  0  0  CIP: Drain Boil-kettle 1  0x012C
    // State 25: 0  1  0  0  1  0  0  1  0  0  CIP: Drain Boil-kettle 2  0x0124
    // State 26: 0  0  0  0  0  0  0  0  0  0  CIP: Fill HLT             0x0000
    // State 27: 0  1  0  1  0  0  0  0  1  0  CIP: Clean Output V7      0x0142
    // State 28: 0  1  0  0  1  0  0  0  1  0  CIP: Clean Output V6      0x0122
    // State 29: 0  1  0  0  0  0  1  0  1  0  CIP: Clean Output V4      0x010A
    // State 30: 0  0  0  0  0  0  0  1  1  0  CIP: Clean Input V3       0x0006
    // State 31: 0  0  0  0  0  0  0  0  1  1  CIP: Clean Input V1       0x0003
    // State 32: 0  0  0  0  0  0  0  0  0  0  CIP: End                  0x0000
 //----------------------------------------------------------------------------
    uint16_t  actuatorSettings[] = {0x0000, 0x0200, 0x030B, 0x0309, 0x0309, /* 04 */
                           /* 05 */ 0x0309, 0x0141, 0x030B, 0x0000, 0x0141, /* 09 */
                           /* 10 */ 0x0000, 0x0000, 0x0000, 0x0200, 0x0003, /* 14 */
                           /* 15 */ 0x0000, 0x0124, 0x0000, 0x0000, 0x0309, /* 19 */
                           /* 20 */ 0x0000, 0x016C, 0x016C, 0x0000, 0x012C, /* 24 */
                           /* 25 */ 0x0124, 0x0000, 0x0142, 0x0122, 0x010A, /* 29 */
                           /* 30 */ 0x0006, 0x0003, 0x0000}; /* 32 */

    bool      maltAdded; // help var. in state S01_WAIT_FOR_HLT_TEMP
    QString   string;    // For std_text->setText()
    QString   substring; // For std_text->setSubText()

    switch (ebrew_std)
    {
        //---------------------------------------------------------------------------
        // S00_INITIALISATION: system off, wait for PID controller to be switched on
        // - If PID is enabled, then goto S01_WAIT_FOR_HLT_TEMP
        //---------------------------------------------------------------------------
        case S00_INITIALISATION:
            string    = QString("00. Initialisation");
            substring = QString("Press the HLT PID Controller button to advance to the next state");
            ms_idx    = 0;                       // init. index in mash schem
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-kettle
            boil_pid->setButtonState(false);     // Disable PID-Controller button for Boil-kettle
            if (hlt_pid->getButtonState())       // Is PowerButton pressed for HLT PID controller?
            {  // start with normal brewing states
                ebrew_std = S01_WAIT_FOR_HLT_TEMP;
            } // if
            else if (toolStartCIP->isChecked())  // Is Start CIP checkbox checked at top toolbar?
            {  // Clean-in-Place program
               setTopToolBar(TOOLBAR_CIP);         // switch to CIP top-toolbar
               toolCipInitDone->setEnabled(true);  // enable Checkbox at toolbar top
               toolCipInitDone->setChecked(false); // uncheck Checkbox
               msgBox("Cleaning in Place (CIP) Initialisation",
                      "1) Fill Boil-kettle with a 1% NaOH solution.\n"
                      "2) Place MLT top return-pipe into Boil-kettle\n"
                      "3) Place CFC output-hose into Boil-kettle\n\n"
                      "Press OK to continue",toolCipInitDone);
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
            string    = QString("01. Wait for HLT Temperature (%1 °C)").arg(tset_hlt,2,'f',1);
            substring = QString("HLT is heated to the first mash-scheme temp.");
            toolStartCIP->setEnabled(false);     // Hide CIP option at toolbar
            if (RegEbrew->value("CB_Malt_First").toInt() > 0)
            {   // Enable 'Malt is added' checkbox at top-toolbar
                toolMaltAdded->setEnabled(true);
            } // if
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-kettle
            maltAdded = (RegEbrew->value("CB_Malt_First").toInt() == 0) || toolMaltAdded->isChecked();
            if ((thlt >= tset_hlt) && maltAdded)
            {   // HLT TEMP is OK and malt is added when MaltFirst option is selected
                if (toolMaltAdded->isChecked()) toolMaltAdded->setEnabled(false); // disable checkbox, no longer needed
                Vhlt_old  = Vhlt; // remember old value
                timer3    = 0;    // init. '1 minute' timer
                ebrew_std = S14_PUMP_PREFILL;
            } // if
            else if (!maltAdded)
            {
                string.append(" + Add Malt to MLT (M)");
                substring.append(", click \'Malt added to MLT\' at top toolbar if malt is added");
            } // else
            break;

        //---------------------------------------------------------------------------
        // S14_PUMP_PREFILL: since the pump needs to be filled with water, prefill
        // the pump with water for a minute, then goto S02_FILL_MLT
        //---------------------------------------------------------------------------
        case S14_PUMP_PREFILL:
            string    = QString("14. Pump Pre-fill (%1/%2 sec.)").arg(timer3).arg(TMR_PREFILL_PUMP);
            substring = QString("The pump needs to be primed for 1 minute, before being switched on");
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-kettle
            if (++timer3 >= TMR_PREFILL_PUMP)    // Stay-here timer timeout?
            {
                timer3    = 0; // reset timer
                ebrew_std = S02_FILL_MLT;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S02_FILL_MLT: Thlt >= tset_HLT, ready to fill MLT with water from HLT
        // - Set pump on
        // - If Vmlt > dough-in volume, goto S03_MASH_IN_PROGRESS
        //---------------------------------------------------------------------------
        case S02_FILL_MLT:
            string    = QString("02. Fill MLT with %1 L water").arg(mash_vol);
            substring = QString("MLT needs to be filled with the required amount of mash water");
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_boil = 0.0;                     // Setpoint Temp. for Boil-kettle
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
            string    = QString("03. Wait for MLT Temperature (%1 °C)").arg(ms[ms_idx].temp + RegEbrew->value("TOffset2").toDouble(),2,'f',1);
            substring = QString("If the MLT Temperature is correct, the mashing phase is started");
            // Add double offset as long as Tmlt < Tset_mlt + Offset2
            tset_mlt  = ms[ms_idx].temp; // get temp. from mash-scheme
            // tset_hlt is NOT set here, but in previous state (FILL_MLT or MASH_PREHEAT_HLT)
            tset_boil = 0.0;             // Setpoint Temp. for Boil-kettle
            if (tmlt >= ms[ms_idx].temp + RegEbrew->value("TOffset2").toDouble())
            {  // Tmlt has reached Tset_mlt + Offset2, start mash timer
                if ((ms_idx == 0) && (RegEbrew->value("CB_Malt_First").toInt() == 0))
                {   // We need to add malt to the MLT first
                    // Depending on the state of the checkbox 'Leave pumps running',
                    // we go directly to state 15 or only after the user selects the
                    // 'Start adding malt' checkbox in the top-toolbar
                    toolStartAddMalt->setEnabled(true);   // enable checkbox 'Start Adding Malt' at top-toolbar
                    if (RegEbrew->value("CB_pumps_on").toInt())
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
            string    = QString("19. Ready to add Malt to MLT (M)");
            substring = QString("If ready to add malt, click \'Start adding Malt\' on toolbar at top of screen");
            if (toolStartAddMalt->isChecked()) ebrew_std = S15_ADD_MALT_TO_MLT;
            break;

        //---------------------------------------------------------------------------
        // S15_ADD_MALT_TO_MLT: Before a mash-timer is started, the malt has to be
        // added to the MLT first. This is done only once (when ms_idx == 0). If this
        // is done, then goto S04_MASH_TIMER_RUNNING.
        //---------------------------------------------------------------------------
        case S15_ADD_MALT_TO_MLT:
            string    = QString("15. Add Malt to MLT (M)");
            substring = QString("If malt is added, click \'Malt added to MLT\' at top toolbar");
            toolStartAddMalt->setEnabled(false); // disable checkbox, no longer needed
            toolMaltAdded->setEnabled(true);     // enable checkbox 'Malt added to MLT'
            if (toolMaltAdded->isChecked())
            {  // malt is added to MLT, start mash timer
               ms[ms_idx].timer = 0; // start the corresponding mash timer
               if (RegEbrew->value("CB_Mash_Rest").toInt() == 1)
               {   // Start with mash rest for 5 min. after malt is added
                   mrest_tmr = 0; // init mash rest timer
                   ebrew_std = S18_MASH_REST_5_MIN;
               } // if
               else ebrew_std = S04_MASH_TIMER_RUNNING;
               toolMaltAdded->setEnabled(false); // disable checkbox, no longer needed
            } // if
            break;

        //---------------------------------------------------------------------------
        // S04_MASH_TIMER_RUNNING: Tmlt has reached Tset_mlt (+Offset2)
        // - Tset_hlt = tset (from mash scheme) + single offset
        // - Increment mash timer until time-out
        // - If more mash phases are required, goto MASH_PREHEAT_HLT else goto SPARGE
        //---------------------------------------------------------------------------
        case S04_MASH_TIMER_RUNNING:
            string = QString("04. Mash-Timer Running (%1/%2 min.)").arg(ms[ms_idx].timer/60).arg(ms[ms_idx].time/60.0,1,'f',0);
            substring = QString("After timeout, HLT is heated to the next temperature");
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble();  // Single offset
            tset_boil = 0.0; // Setpoint Temp. for Boil-kettle
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
            string    = QString("13. Mash Preheat HLT (%1/%2 min.)").arg(ms[ms_idx].timer/60).arg(ms[ms_idx].time/60.0,2,'f',0);
            substring = QString("The HLT is being preheated to the next mash-temperature");
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = ms[ms_idx + 1].temp + 2 * RegEbrew->value("TOffset").toDouble();
            tset_boil = 0.0; // Setpoint Temp. for Boil-kettle
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
            string    = QString("18. Mash-Rest (%1/%2 sec.)").arg(mrest_tmr).arg(TMR_MASH_REST_5_MIN);
            substring = QString("Option Mash-Rest is active, waiting 5 minutes");
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            tset_boil = 0.0; // Setpoint Temp. for Boil-kettle
            if (ms_idx < ms_tot - 1)
            {  // There's a next mash phase
                if (ms[ms_idx].timer >= ms[ms_idx].preht)
                {  // Preheat timer has priority, since it also switches off the pump
                    ebrew_std = S13_MASH_PREHEAT_HLT;
                } // if
                else if ((RegEbrew->value("CB_Mash_Rest").toInt() == 0) || (++mrest_tmr >= TMR_MASH_REST_5_MIN))
                {  // after 5 min. of mash-rest, goto normal mashing phase and switch pump on
                    mrest_tmr = 0; // reset mrest_tmr
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
            string    = QString("05. Sparge-Timer Running (%1/%2 min.)").arg(timer1/60).arg(RegEbrew->value("SP_TIME").toInt());
            substring = QString("After timeout, wort is pumped to the Boil-kettle");
            tset_mlt = ms[ms_idx].temp;
            tset_hlt = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = 0.0;
            if (++timer1 >= sp_time_ticks)
            {
                Vmlt_old  = Vmlt;  // save Vmlt  for states 6 & 9
                Vboil_old = Vboil; // save Vboil for states 6 & 9
                if (sp_idx < RegEbrew->value("SP_BATCHES").toInt())
                {
                    mlt2boil << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                    ebrew_std = S06_PUMP_FROM_MLT_TO_BOIL; // Pump to BOIL (again)
                } // if
                else
                {  // Init flowrate-low detector for flow2 mlt -> hlt
                    F2->initFlowRateDetector(RegEbrew->value("MIN_FR_MLT_PERC").toInt());
                    timer1    = 0;                  // reset timer1
                    ebrew_std = S09_EMPTY_MLT;      // Finished with Sparging, empty MLT
                    toolMLTEmpty->setEnabled(true); // enable checkbox at top-toolbar
                } // else if
            } // if
            break;

        //---------------------------------------------------------------------------
        // S06_PUMP_FROM_MLT_TO_BOIL:
        // - Pump wort from MLT to Boil kettle until Vmlt change > sparge batch size
        // - The goto S08_DELAY_xSEC
        //---------------------------------------------------------------------------
        case S06_PUMP_FROM_MLT_TO_BOIL:
            string    = QString("06. Pump from MLT to Boil-kettle (%1 L)").arg(sp_idx ? sp_vol_batch : sp_vol_batch0,2,'f',1);
            substring = QString("Wort is pumped to the Boil-kettle");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
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
            string    = QString("07. Pump fresh water from HLT to MLT (%1 L)").arg(sp_vol_batch,2,'f',1);
            substring = QString("A batch of fresh sparge water is added from the HLT");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
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
            string    = QString("08. Delay: %1 seconds").arg(TMR_DELAY_xSEC);
            substring = QString("Short delay of 10 seconds");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boil_pid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = 0.0;

            if (++timer2 >= TMR_DELAY_xSEC)
            {
                hlt2mlt << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                Vhlt_old  = Vhlt;  // remember old value
                Vmlt_old  = Vmlt;  // remember current MLT volume
                timer2    = 0;     // reset timer2
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
            string    = QString("09. Empty MLT");
            substring = QString("All remaining wort from the MLT is pumped to the Boil-kettle");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = 0.0;                // Disable HLT PID-Controller
            hlt_pid->setButtonState(false); // Disable PID-controller for HLT
            tset_boil = RegEbrew->value("SP_BOIL").toDouble();  // Boil Temperature Setpoint
            boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            if (toolMLTEmpty->isChecked() || F2->isFlowRateLow())
            {
                ebrew_std = S10_WAIT_FOR_BOIL;
                timer5    = 0; // assure a min. stay time, so transition is detected
                toolMLTEmpty->setChecked(true);  // Set checkbox to checked
                toolMLTEmpty->setEnabled(false); // ... and disable it, no longer needed
            } // if
            break;

        //---------------------------------------------------------------------------
        // S10_WAIT_FOR_BOIL: After all wort is pumped to the boil kettle, it takes
        // a while before boiling actually starts. When the user selects the
        // 'Boiling Started' option or when the Boil-kettle temperature exceeds
        // the minimum temperature BOIL_DETECT, goto S11_BOILING.
        //---------------------------------------------------------------------------
        case S10_WAIT_FOR_BOIL:
            string    = QString("10. Waiting for Boil (M)");
            substring = QString("If boiling is not detected automatically, click \'Boiling Started\' at top toolbar");
            tset_hlt  = 0.0; // disable heating element
            tset_boil = RegEbrew->value("SP_BOIL").toDouble(); // Boil Temperature Setpoint
            toolBoilStarted->setEnabled(true); // Enable checkbox at top-toolbar
            boil_pid->setButtonState(true);    // Enable PID-Controller for Boil-kettle
            if (toolBoilStarted->isChecked() || (tboil > RegEbrew->value("BOIL_DETECT").toDouble()))
            {
                toolBoilStarted->setChecked(true);       // Set checkbox to checked
                toolBoilStarted->setEnabled(false);      // ... and disable again, no longer needed
                Boil << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                timer5    = 0;                           // init. timer for boiling time
                ebrew_std = S11_BOILING;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S11_BOILING: Boiling has started, remain here for BOIL_TIME minutes
        //---------------------------------------------------------------------------
        case S11_BOILING:
            string    = QString("11. Now Boiling (%1/%2 min.)").arg(timer5/60).arg(boil_time);
            substring = QString("Now boiling");
            tset_boil = RegEbrew->value("SP_BOIL").toDouble(); // Boil Temperature Setpoint
            boil_pid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            if (++timer5 >= boil_time_ticks)
            {
                toolStartChilling->setEnabled(true);     // Enable checkbox at top-toolbar
                Boil << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                FlowCfcOutResetValue = FlowCfcOut;       // reset Flow_cfc_out
                brest_tmr = 0; // init boil-rest timer
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
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-kettle
            if (((RegEbrew->value("CB_Boil_Rest").toInt() == 0) || (++brest_tmr > TMR_BOIL_REST_5_MIN)) && toolStartChilling->isChecked())
            {  // Init flow3 (cfc-out) flowrate-low detector
                F3->initFlowRateDetector(RegEbrew->value("MIN_FR_BOIL_PERC").toInt());
                toolStartChilling->setChecked(true);      // Set checkbox to checked
                toolStartChilling->setEnabled(false);     // ... and disable again, no longer needed
                toolReadyChilling->setEnabled(true);      // Enable checkbox 'Chilling finished'
                Chill << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                ebrew_std = S16_CHILL_PUMP_FERMENTOR;
            } // if
            else if (RegEbrew->value("CB_Boil_Rest").toInt() == 1)
            {
               if (brest_tmr > TMR_BOIL_REST_5_MIN)
                    string = QString("12. Boiling Finished, prepare Chiller (M)");
               else string = QString("12. Boiling Finished, wait %1/%2 min., prepare Chiller (M)").arg(brest_tmr/60).arg(TMR_BOIL_REST_5_MIN/60);
            } // if
            else string = QString("12. Boiling Finished, prepare Chiller (M)");
            substring   = QString("Prepare chiller. If ready, click \'CFC Prepared, start Chilling\' at top toolbar");
            break;

        //---------------------------------------------------------------------------
        // S16_CHILL_PUMP_FERMENTOR: The boiled wort is sent through the counterflow
        // chiller and directly into the fermentation bin.
        //---------------------------------------------------------------------------
        case S16_CHILL_PUMP_FERMENTOR:
            string    = QString("16. Chill & Pump to Fermentation Bin (M)");
            substring = QString("If end of chilling is not detected automatically, click \'Chilling is finished\' at top toolbar");
            tset_boil = 0.0;  // Boil Temperature Setpoint
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-kettle
            if (toolReadyChilling->isChecked() || F3->isFlowRateLow()) // flowRate of CFC-output
            {
                toolReadyChilling->setChecked(true);      // Set checkbox to checked
                toolReadyChilling->setEnabled(false);     // ... and disable it, no longer needed
                Chill << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                ebrew_std = S17_FINISHED;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S17_FINISHED: All wort in pumped into the fermentation bin. The brewing
        // session is finished. This is the end-state.
        //---------------------------------------------------------------------------
        case S17_FINISHED:
            string    = QString("17. Finished!");
            substring = QString("You need to close and restart this program for a new brew session");
            tset_boil = 0.0;  // Boil Temperature Setpoint
            boil_pid->setButtonState(false); // Disable PID-Controller for Boil-kettle
            // Remain in this state until Program Exit.
            break;

        //---------------------------------------------------------------------------
        //               C L E A N I N G    I N    P L A C E   ( C I P )
        //---------------------------------------------------------------------------
        // S20_CIP_INIT: Start of Cleaning-In-Place Program.
        // Wait until Boil-kettle has been filled with 1% NaOH solution.
        //---------------------------------------------------------------------------
        case S20_CIP_INIT:
            string    = QString("20. CIP: Initialisation, fill Boil-kettle with 1% NaOH");
            substring = QString("Put MLT-return & CFC-output in Boil-kettle, click \'CIP init. done\' at top toolbar");
            tset_hlt  = 0.0; // HLT setpoint temperature
            tset_boil = 0.0; // Boil-kettle setpoint temperature
            cip_circ  = 0;   // Init. CIP circulation counter
            boil_pid->setButtonState(false);   // Disable PID-Controller for Boil-kettle
            if (toolCipInitDone->isChecked())  // User indicated that Boil-kettle is filled
            {
                toolCipInitDone->setEnabled(false); // disable checkbox, no longer needed
                ebrew_std = S21_CIP_HEAT_UP;
            } // if
            else if (!toolStartCIP->isChecked())
            {
                toolCipInitDone->setEnabled(false); // disable Checkbox at toolbar top
                setTopToolBar(TOOLBAR_BREWING);     // select normal brewing toolbar at top of screen
                ebrew_std = S00_INITIALISATION;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S21_CIP_HEAT_UP: Heat Boil-kettle up to predefined temperature.
        //---------------------------------------------------------------------------
        case S21_CIP_HEAT_UP:
             string    = QString("21. CIP: Heat-up and circulate");
             substring = QString("NaOH solution is pomped through while heated to setpoint temperature");
             tset_boil = RegEbrew->value("CIP_SP").toDouble(); // Boil-kettle Temperature Setpoint
             PidCtrlBk->pidEnable(PID_FFC);  // Enable Feed-forward control for Boil-kettle
             boil_pid->setButtonState(true); // Enabled PID-Controller Power-button
             if (tboil > tset_boil - 5.0)    // Almost at setpoint temperature
             {
                cip_tmr1  = 0;        // Init. CIP timer
                ebrew_std = S22_CIP_CIRC_5_MIN;
             } // if
             else if (!toolStartCIP->isChecked())
             {
                setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                ebrew_std = S00_INITIALISATION;
             } // else
             break;

         //---------------------------------------------------------------------------
         // S22_CIP_CIRC_5_MIN: Circulate NaOH solution through brewing system pipes
         //---------------------------------------------------------------------------
         case S22_CIP_CIRC_5_MIN:
              string    = QString("22. CIP: Circulating (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_CIRC_TIME").toInt());
              substring = QString("NaOH solution is circulated through all pipes");
              tset_boil = RegEbrew->value("CIP_SP").toDouble(); // Boil-kettle Temperature Setpoint
              PidCtrlBk->pidEnable(PID_ON);      // Enable normal control for Boil-kettle
              boil_pid->setButtonState(true);    // Enable PID-Controller Power-button
              if (++cip_tmr1 >= RegEbrew->value("CIP_CIRC_TIME").toInt())
              {
                 cip_tmr1  = 0;        // Reset CIP timer
                 if (++cip_circ > 1)   // Count number of CIP circulations
                 {
                    toolCipDrainBK->setEnabled(true);  // enable Checkbox at toolbar top
                    toolCipDrainBK->setChecked(false); // uncheck Checkbox
                    msgBox("Cleaning in Place (CIP): Drain Boil-kettle",
                           "1) Place MLT top return-pipe into drain\n"
                           "2) Place CFC output-hose into drain\n\n"
                           "Press OK to continue",toolCipDrainBK);
                    ebrew_std = S24_CIP_DRAIN_BOIL1;
                 }	// if
                 else ebrew_std = S23_CIP_REST_5_MIN;
              } // if
              else if (!toolStartCIP->isChecked())
              {
                 setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                 ebrew_std = S00_INITIALISATION;
              } // else
              break;

          //---------------------------------------------------------------------------
          // S23_CIP_REST_5_MIN: Rest period, all valves and pump off
          //---------------------------------------------------------------------------
          case S23_CIP_REST_5_MIN:
               string    = QString("23. CIP: Resting (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_REST_TIME").toInt());
               substring = QString("Pipes are being cleaned with the NaOH solution");
               tset_boil = RegEbrew->value("CIP_SP").toDouble(); // Boil-kettle Temperature Setpoint
               boil_pid->setButtonState(true);    // Enable PID-Controller for Boil-kettle
               if (++cip_tmr1 >= RegEbrew->value("CIP_REST_TIME").toInt())
               {
                  cip_tmr1  = 0;        // Reset CIP timer
                  ebrew_std = S22_CIP_CIRC_5_MIN;
               } // if
               else if (!toolStartCIP->isChecked())
               {
                  setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                  ebrew_std = S00_INITIALISATION;
               } // else
               break;

           //---------------------------------------------------------------------------
           // S24_CIP_DRAIN_BOIL1: Empty Boil-kettle, remove NaOH solution. Do this by
           //                      placing the CFC-output and MLT-return hoses in the drain.
           //---------------------------------------------------------------------------
           case S24_CIP_DRAIN_BOIL1:
                string    = QString("24. CIP: Drain Boil-kettle 1");
                substring = QString("NAOH solution is removed from the Boil-kettle");
                tset_boil = 0.0; // Boil-kettle Temperature Setpoint
                boil_pid->setButtonState(false); // Disable PID-Controller for Boil-kettle
                if (toolCipDrainBK->isChecked())
                {  // Init flowrate-low detector
                   F3->initFlowRateDetector(RegEbrew->value("MIN_FR_BOIL_PERC").toInt());
                   ebrew_std = S25_CIP_DRAIN_BOIL2;
                } // if
                else if (!toolStartCIP->isChecked())
                {
                   setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                   ebrew_std = S00_INITIALISATION;
                } // else
                break;

            //---------------------------------------------------------------------------
            // S25_CIP_DRAIN_BOIL2: Empty Boil-kettle, remove NaOH solution. Check when
            //                      CFC-output flowrate is low, then Boil-kettle is empty.
            //---------------------------------------------------------------------------
            case S25_CIP_DRAIN_BOIL2:
                 string    = QString("25. CIP: Drain Boil-kettle 2");
                 substring = QString("NAOH solution is removed, now fill HLT with fresh water");
                 tset_boil = 0.0; // Boil-kettle Temperature Setpoint
                 boil_pid->setButtonState(false); // Disable PID-Controller for Boil-kettle
                 if (F3->isFlowRateLow()) // flowrate of CFC-output
                 {
                     toolCipHltFilled->setEnabled(true);  // enable Checkbox at toolbar top
                     toolCipHltFilled->setChecked(false); // uncheck Checkbox
                     msgBox("Cleaning in Place (CIP): Fill HLT with Fresh Water",
                            "1) NEW: Fill HLT with fresh water\n"
                            "2) NEW: Place Boil-kettle return into drain\n"
                            "3) Leave MLT top return-pipe into drain\n"
                            "4) Leave CFC output-hose into drain\n\n"
                            "Press OK to continue",toolCipHltFilled);
                     ebrew_std = S26_CIP_FILL_HLT;
                 } // if
                 else if (!toolStartCIP->isChecked())
                 {
                    setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                    ebrew_std = S00_INITIALISATION;
                 } // else
                 break;

             //---------------------------------------------------------------------------
             // S26_CIP_FILL_HLT: Fill HLT with sufficient fresh water
             //---------------------------------------------------------------------------
             case S26_CIP_FILL_HLT:
                  string    = QString("26. CIP: Fill HLT with fresh water");
                  substring = QString("Fill HLT with fresh water, then continue");
                  if (toolCipHltFilled->isChecked())
                  {  // User indicated that HLT has been filled with fresh water
                     cip_tmr1  = 0;
                     ebrew_std = S27_CIP_CLEAN_OUTPUT_V7;
                  } // if
                  else if (!toolStartCIP->isChecked())
                  {
                     setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                     ebrew_std = S00_INITIALISATION;
                  } // else
                  break;

        //---------------------------------------------------------------------------
        // S27_CIP_CLEAN_OUTPUT_V7: Clean output V7 of brewing system with fresh water
        //---------------------------------------------------------------------------
        case S27_CIP_CLEAN_OUTPUT_V7:
             string    = QString("27. CIP: Clean Output V7");
             substring = QString("Output V7 (Boil-kettle input) is being cleaned");
             if (++cip_tmr1 >= RegEbrew->value("CIP_OUT_TIME").toInt())
             {
                cip_tmr1  = 0; // Reset CIP timer
                ebrew_std = S28_CIP_CLEAN_OUTPUT_V6;
             } // if
             else if (!toolStartCIP->isChecked())
             {
                setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                ebrew_std = S00_INITIALISATION;
             } // else
             break;

         //---------------------------------------------------------------------------
         // S28_CIP_CLEAN_OUTPUT_V6: Clean output V6 of brewing system with fresh water
         //---------------------------------------------------------------------------
         case S28_CIP_CLEAN_OUTPUT_V6:
              string    = QString("28. CIP: Clean Output V6");
              substring = QString("Output V6 (CFC-output) is being cleaned");
              if (++cip_tmr1 >= RegEbrew->value("CIP_OUT_TIME").toInt())
              {
                 cip_tmr1  = 0; // Reset CIP timer
                 ebrew_std = S29_CIP_CLEAN_OUTPUT_V4;
              } // if
              else if (!toolStartCIP->isChecked())
              {
                 setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                 ebrew_std = S00_INITIALISATION;
              } // else
              break;

          //---------------------------------------------------------------------------
          // S29_CIP_CLEAN_OUTPUT_V4: Clean output V4 of brewing system with fresh water
          //---------------------------------------------------------------------------
          case S29_CIP_CLEAN_OUTPUT_V4:
               string    = QString("29. CIP: Clean Output V4");
               substring = QString("Output V4 (MLT top return manifold) is being cleaned");
               if (++cip_tmr1 >= RegEbrew->value("CIP_OUT_TIME").toInt())
               {
                  cip_tmr1  = 0; // Reset CIP timer
                  ebrew_std = S30_CIP_CLEAN_INPUT_V3;
               } // if
               else if (!toolStartCIP->isChecked())
               {
                  setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                  ebrew_std = S00_INITIALISATION;
               } // else
               break;

           //---------------------------------------------------------------------------
           // S30_CIP_CLEAN_INPUT_V3: Clean input V3 of brewing system with fresh water.
           //                         This is done by gravity-feed, not with a pump.
           //---------------------------------------------------------------------------
           case S30_CIP_CLEAN_INPUT_V3:
                string    = QString("30. CIP: Clean Input V3");
                substring = QString("Input V3 (Boil-kettle output) is being cleaned");
                if (++cip_tmr1 >= RegEbrew->value("CIP_INP_TIME").toInt())
                {
                   cip_tmr1  = 0; // Reset CIP timer
                   ebrew_std = S31_CIP_CLEAN_INPUT_V1;
                } // if
                else if (!toolStartCIP->isChecked())
                {
                   setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                   ebrew_std = S00_INITIALISATION;
                } // else
                break;

            //---------------------------------------------------------------------------
            // S31_CIP_CLEAN_INPUT_V1: Clean input V1 of brewing system with fresh water.
            //                         This is done by gravity-feed, not with a pump.
            //---------------------------------------------------------------------------
            case S31_CIP_CLEAN_INPUT_V1:
                 string    = QString("31. CIP: Clean Input V1");
                 substring = QString("Input V1 (MLT output) is being cleaned");
                 if (++cip_tmr1 >= RegEbrew->value("CIP_INP_TIME").toInt())
                 {
                    ebrew_std = S32_CIP_END;
                 } // if
                 else if (!toolStartCIP->isChecked())
                 {
                    setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                    ebrew_std = S00_INITIALISATION;
                 } // else
                 break;

            case S32_CIP_END:
                 string    = QString("32. CIP: End of CIP-program");
                 substring = QString("Uncheck checkbox \'Start Clean-In-Place (CIP)\' to return to Init. state");
                 if (!toolStartCIP->isChecked())
                 {
                    setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                    commPortWrite("R0");            // reset all flows to 0.0 L in Ebrew hardware
                    ebrew_std = S00_INITIALISATION;
                 } // else
                 break;

        //---------------------------------------------------------------------------
        // Default: should never get here
        //---------------------------------------------------------------------------
        default:
                string    = QString("xx. Unknown State");
                substring = QString("Internal error, the STD is not supposed to get here.");
                setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                ebrew_std = S00_INITIALISATION;
            break;
    } // switch

    std_text->setText(string);       // Update STD label
    std_text->setSubText(substring); // Update STD sub-text

    //-------------------------------------------------
    // Now calculate the proper settings for the valves
    //-------------------------------------------------
    uint16_t actuators = actuatorSettings[ebrew_std];
    P2->setActuator(actuators,P1b); // Main pump
    P1->setActuator(actuators,P0b); // Pump for HLT heat-exchanger
    // V8: Future use
    V7->setActuator(actuators,V7b); // Input valve for Boil-kettle
    V6->setActuator(actuators,V6b); // Output valve for CFC
    // V5: Future use
    V4->setActuator(actuators,V4b); // Valve between HLT heat-exchanger and MLT return at top
    V3->setActuator(actuators,V3b); // Output valve of Boil-kettle
    V2->setActuator(actuators,V2b); // Output valve of HLT
    V1->setActuator(actuators,V1b); // Output valve of MLT
    return actuators;
} // MainEbrew::state_machine()
