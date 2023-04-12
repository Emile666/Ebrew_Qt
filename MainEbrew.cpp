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
#include "dialogeditterminal.h"
#include "dialogviewprogress.h"
#include "dialogviewtasklist.h"
#include "dialogviewstatusalarms.h"
#include "dialogoptionspidsettings.h"
#include "dialogoptionsmeasurements.h"
#include "dialogbrewdaysettings.h"
#include "dialogoptionssystemsettings.h"

//------------------------------------------------------------------------------------------
// CLASS MainEbrew
//------------------------------------------------------------------------------------------
// TODO Add Help-file
// TODO Switch between English/Dutch language

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
    splitIpAddressPort();            // Split Registry IP-address and port-number
    readMashSchemeFile(INIT_TIMERS); // Read mash scheme from file and init. all mash timers
    initBrewDaySettings();           // Init. mash, sparge and boil setting with values from Registry
    toolHLTPilotLight->setEnabled(RegEbrew->value("HEATERSH").toInt() & GAS_MODULATING); // Enable pilot-light checkbox
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
        ebrewHwIp = QHostAddress::AnyIPv4;
        QHostAddress tempIp = ipAddress; // save ipAddress
        ipAddress = QHostAddress::Broadcast; // comPortWrite() generates broadcast on subnet
        commPortWrite("S0"); // retry
        sleep(100);          // wait until data available
        if (ReadDataAvailable)
        {
            found = (ReadData.indexOf(EBREW_HW_ID,0) != -1);
            ReadDataAvailable = false;
            //qDebug() << "Found(" << found << "): " << ReadData << "IP:" << ebrewHwIp;
        } // if
        ipAddress = tempIp; // restore IP-address from Registry
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
        QString s = statusHops->text().remove(0,5);
        s.insert(0,"Hop-additions    : ");
        stream << s << "\n";
        stream << "Comm. channel    : ";
        int x = RegEbrew->value("COMM_CHANNEL").toInt();
        if (x > 0)
             stream << QString("COM%1").arg(x);
        else stream << RegEbrew->value("UDP_IP_PORT").toString();
        stream << "\n";
        stream << line1MashScheme << "\n";
        stream << " Time    TsetM TsetH  Thlt  Tmlt Telc  Vmlt s m st  GmaH  Vhlt VBoil TBoil  Tcfc GmaB\n";
        stream << "[h:m:s]   [\xB0""C]  [\xB0""C]  [\xB0""C]  [\xB0""C] [\xB0""C]   [L] p s  d   [%]   [L]   [L]  [\xB0""C]  [\xB0""C]  [%]\n";
        stream << "-------------------------------------------------------------------------------------\n";
    } // if
    else fEbrewLog = nullptr;
    commPortWrite("R0"); // reset all flows to 0.0 L in ebrew hardware
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
    int ret = QMessageBox::Ok;
    if ((ebrew_std > S00_INITIALISATION) && (ebrew_std != S17_FINISHED))
    {
        QMessageBox msgBox;
        msgBox.setText("<span style='text-align: center'><p style='font-size: 11pt;'><b>A Brewing session is in progress...     <b></p></span>");
        msgBox.setInformativeText("Are you sure you want to quit?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Question);
        ret = msgBox.exec();
    } // if
    if (ret == QMessageBox::Ok)
    {
        schedulerEbrew->stop(); // stop all tasks

        commPortWrite("B0 0"); // Disable Boil-kettle gas-burner
        commPortWrite("H0 0"); // Disable HLT energy-sources
        commPortWrite("L0"); // Disable Alive-LED
        commPortWrite("P0"); // Disable Pump
        commPortWrite("V0"); // Disable All Valves
        sleep(100);          // Give comm. channel some time to send it

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
    } // if
    else event->ignore(); // Do not accept, continue with Ebrew program
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
    statusAlarm->setToolTip("Shows which sensor error results in an audible alarm (one or more beeps). Use \'<b>S</b>\' to cycle through the following options:<br><b>OFF</b>: No audible alarm.<br><b>TEMP</b>: Only alarm if one or more temperature sensors fail.<br><b>FLOW</b>: Only alarm if one or more flow sensors fail.<br><b>TEMP+FLOW</b>: alarm on every sensor failure.");
    statusBar->addPermanentWidget(statusAlarm,1);
    statusMashScheme = new QLabel("     ");
    statusMashScheme->setAlignment(Qt::AlignHCenter);
    statusMashScheme->setToolTip("Shows the Mash Scheme read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusMashScheme,1);
    statusHops = new QLabel("");
    statusHops->setAlignment(Qt::AlignHCenter);
    statusHops->setToolTip("Shows the Hops Scheme read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusHops);
    statusMashVol    = new QLabel(" Mash Volume: 0 L ");
    statusMashVol->setAlignment(Qt::AlignHCenter);
    statusMashVol->setToolTip("Shows the mash volume, which is the total amount of water for the MLT used for mashing.<br>Value is read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusMashVol,1);
    statusSpargeVol  = new QLabel(" Sparge Volume: 0 L ");
    statusSpargeVol->setAlignment(Qt::AlignHCenter);
    statusSpargeVol->setToolTip("Shows the sparge volume, which is the total amount of water used for batch-sparging.<br>Value is read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusSpargeVol,1);
    statusBoilTime  = new QLabel(" Boil-time: 0 min. ");
    statusBoilTime->setAlignment(Qt::AlignHCenter);
    statusBoilTime->setToolTip("This is the total boil-time in minutes for the boil-kettle.<br>Value is read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusBoilTime,1);
    statusMsIdx      = new QLabel(" Mash index: 0 ");
    statusMsIdx->setAlignment(Qt::AlignHCenter);
    statusMsIdx->setToolTip("The actual mash scheme index: it points to the current temperature in the mash-scheme, 0 = the first temperature.<br>Value is read from the default Mash Scheme File maisch.sch.<br>See also Edit->Mash Scheme...");
    statusBar->addPermanentWidget(statusMsIdx,1);
    statusSpIdx      = new QLabel(" Sparge index: 0 ");
    statusSpIdx->setAlignment(Qt::AlignHCenter);
    statusSpIdx->setToolTip("The actual sparging index: it points to the current batch-sparge, 0 = the first batch sparge.<br>The total number of batch-sparge can be adjusted with Options->Brew Day Settings->Sparging");
    statusBar->addPermanentWidget(statusSpIdx,1);
    statusSwRev      = new QLabel(" SW r1.99 HW r1.24 ");
    statusSwRev->setAlignment(Qt::AlignHCenter);
    statusSwRev->setToolTip("Shows the software and hardware revision number. The hardware revision number is read during power-up from the Ebrew hardware. If it reads <b>?.?</b>, then no connection could be made to the Ebrew hardware.<br>See also Options->System Settings->Communications");
    statusBar->addPermanentWidget(statusSwRev,1);
    setStatusBar(statusBar); // connect the statusBar to Ebrew

    // Create Toolbar1 at top of screen: always visible
    auto toolBar1 = new QToolBar("Toolbar1"); // Toolbar at left of screen
    toolBar1->setMovable(false); // fixed at top of screen
    toolBar1->setOrientation(Qt::Horizontal);
    toolStartCIP = new QCheckBox("Start Clean-In-Place (CIP)");
    toolStartCIP->setToolTip("Enable this checkbox to start cleaning of the brew system, disable to resume normal operation.");
    toolBar1->addWidget(toolStartCIP);
    addToolBar(Qt::TopToolBarArea,toolBar1);

    // Create Toolbar2 at top of screen: brewing checkboxes
    auto toolBar2 = new QToolBar("Toolbar2"); // Toolbar at left of screen
    toolBarB = toolBar2; // save reference in MainEbrew
    toolBar2->setMovable(false); // fixed at top of screen
    toolBar2->setOrientation(Qt::Horizontal);

    toolStartAddMalt = new QCheckBox("Start Adding Malt");
    toolStartAddMalt->setEnabled(false); // default not enabled
    toolStartAddMalt->setToolTip("Enable this checkbox if you are ready to start adding malt to the MLT (the pump is then switched off).");
    toolBar2->addWidget(toolStartAddMalt);
    toolMaltAdded = new QCheckBox("Malt added to MLT");
    toolMaltAdded->setEnabled(false); // default not enabled
    toolMaltAdded->setToolTip("Enable this checkbox if all malt has been added to the MLT and mashing can continu");
    toolBar2->addWidget(toolMaltAdded);
    toolMLTEmpty = new QCheckBox("MLT is empty");
    toolMLTEmpty->setEnabled(false); // default not enabled
    toolMLTEmpty->setToolTip("Enable this checkbox only if the brew system does not detect automatically that the MLT is empty.<br>It does this by monitoring flow sensor 2, in case of a failure, this checkbox can be used instead.");
    toolBar2->addWidget(toolMLTEmpty);
    toolBoilStarted = new QCheckBox("Boiling Started");
    toolBoilStarted->setEnabled(false); // default not enabled
    toolBoilStarted->setToolTip("Enable this checkbox only if the brew system does not detect automatically that boiling has started.<br>It does this by monitoring the boil-kettle temperature sensor. In case of a failure, this checkbox can be used instead.");
    toolBar2->addWidget(toolBoilStarted);
    toolStartChilling = new QCheckBox("CFC Prepared, start Chilling");
    toolStartChilling->setEnabled(false); // default not enabled
    toolStartChilling->setToolTip("Enable this checkbox if the counter flow chiller (CFC) is connected to the water tap for cooling water AND that the CFC output hose is placed in the fermentation bin.");
    toolBar2->addWidget(toolStartChilling);
    toolReadyChilling = new QCheckBox("Chilling is finished");
    toolReadyChilling->setEnabled(false); // default not enabled
    toolReadyChilling->setToolTip("Enable this checkbox only if the brew system does not detect automatically that the Boil-kettle is empty.<br>It does this by monitoring flow sensor 3 at the CFC-output, in case of a failure, this checkbox can be used instead.");
    toolBar2->addWidget(toolReadyChilling);
    toolGFSpargeWater = new QCheckBox("GF Sparge Water Heater");
    toolGFSpargeWater->setToolTip("Enable this checkbox if you want to use the system only as Sparge Water Heater for the GrainFather.");
    toolBar2->addWidget(toolGFSpargeWater);
    toolHLTPilotLight = new QCheckBox("Pilot-light HLT gasburner");
    toolHLTPilotLight->setToolTip("Enable this checkbox if the pilot-light of the HLT gasburner should be ignited.");
    toolBar2->addWidget(toolHLTPilotLight);
    addToolBar(Qt::TopToolBarArea,toolBar2);

    // Create Toolbar3 at top of screen: CIP checkboxes
    auto toolBar3 = new QToolBar("Toolbar3"); // Toolbar at left of screen
    toolBarC = toolBar3; // save reference in MainEbrew
    toolBar3->setMovable(false); // fixed at top of screen
    toolBar3->setOrientation(Qt::Horizontal);
    toolCipInitDone = new QCheckBox("CIP init. done");
    toolCipInitDone->setEnabled(false); // default not enabled
    toolCipInitDone->setToolTip("This checkbox is enabled when the user presses OK in the Dialog Box to indicate that CIP initialisation is done.");
    toolCipDrainBK = new QCheckBox("CIP Drain Boil-kettle");
    toolCipDrainBK->setEnabled(false); // default not enabled
    toolCipDrainBK->setToolTip("This checkbox is enabled when the user presses OK in the Dialog Box to indicate that the Boil-kettle is drained completely.");
    toolCipHltFilled = new QCheckBox("CIP HLT Filled");
    toolCipHltFilled->setEnabled(false); // default not enabled
    toolCipHltFilled->setToolTip("This checkbox is enabled when the user presses OK in the Dialog Box to indicate that the HLT is filled with fresh water.");
    toolBar3->addWidget(toolCipInitDone);
    toolBar3->addWidget(toolCipDrainBK);
    toolBar3->addWidget(toolCipHltFilled);
    addToolBar(Qt::TopToolBarArea,toolBar3);
    setTopToolBar(TOOLBAR_BREWING); // start in normal brewing mode
} // MainEbrew::createStatusBar()

void MainEbrew::updateMsIdxStatusBar(void)
{
    QString sbar;
    sbar = QString(" Mash index: %1 ").arg(ms_idx);
    statusMsIdx->setText(sbar);
} // MainEbrew::updateMsIdxStatusBar()

void MainEbrew::updateSpIdxStatusBar(void)
{
    QString sbar;
    sbar = QString(" Sparge index: %1 ").arg(sp_idx);
    statusSpIdx->setText(sbar);
} // MainEbrew::updateSpIdxStatusBar()

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
    auto menuBar = new QMenuBar;

    // File menu
    auto Fmenu       = new QMenu("&File");
    Fmenu->addAction(QIcon(":/img/fileopen.png")    ,"Read Log-File..."); // TODO slot voor Read logFile
    Fmenu->addSeparator();
    Fmenu->addAction(QIcon(":/img/exit.png")        ,"E&xit"                      , this,SLOT(close())               ,QKeySequence("Ctrl+Q"));
    menuBar->addMenu(Fmenu);
    // Edit menu
    auto Emenu       = new QMenu("&Edit");
    Emenu->addAction(QIcon(":/img/fileedit.png")    ,"&Mash Scheme..."            ,this,SLOT(MenuEditMashScheme())   ,QKeySequence("Ctrl+M"));
    Emenu->addAction(QIcon(":/img/fixparams.png")   ,"&Fix Parameters..."         ,this,SLOT(MenuEditFixParameters()),QKeySequence("Ctrl+F"));
    Emenu->addAction(QIcon(":/img/terminal.png")    ,"Terminal &Editor"           ,this,SLOT(MenuEditTerminal())     ,QKeySequence("Ctrl+E"));
    menuBar->addMenu(Emenu);
    // View menu
    auto Vmenu       = new QMenu("&View");
    Vmenu->addAction(QIcon(":/img/progress.png")    ,"Mash && Sparge &Progress"   ,this,SLOT(MenuViewProgress())     ,QKeySequence("Ctrl+P"));
    Vmenu->addAction(QIcon(":/img/alarm.png")       ,"Status and &Alarms"         ,this,SLOT(MenuViewStatusAlarms()) ,QKeySequence("Ctrl+A"));
    Vmenu->addAction(QIcon(":/img/task.png")        ,"&Task-list and Timings"     ,this,SLOT(MenuViewTaskList())     ,QKeySequence("Ctrl+T"));
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
    RegEbrew->setValue("HEATERSH",0x05); // Modulating gas-burner + 1 heating-element for HLT
    RegEbrew->setValue("HEATERSB",0x05); // Modulating gas-burner + 1 heating-element for Boil-kettle
    RegEbrew->setValue("GAS_NON_MOD_LLIMIT",30); // Parameter 1 for Ebrew HW
    RegEbrew->setValue("GAS_NON_MOD_HLIMIT",35); // Parameter 2 for Ebrew HW
    RegEbrew->setValue("GAS_MOD_PWM_LLIMIT",2);  // Parameter 3 for Ebrew HW
    RegEbrew->setValue("GAS_MOD_PWM_HLIMIT",4);  // Parameter 4 for Ebrew HW
    RegEbrew->setValue("TTRIAC_LLIM",65);        // Parameter 5 for Ebrew HW
    RegEbrew->setValue("TTRIAC_HLIM",75);        // Parameter 6 for Ebrew HW
    // Communications
    RegEbrew->setValue("COMM_CHANNEL",0);                     // Select Ethernet as Comm. Channel
    RegEbrew->setValue("COM_PORT_SETTINGS","38400,N,8,1");    // COM port settings
    RegEbrew->setValue("UDP_IP_PORT","192.168.192.105:8888"); // IP & Port number
    RegEbrew->setValue("CB_DEBUG_COM_PORT",1);                // 1 = log COM port read/writes
    // Brew-kettle Sizes
    RegEbrew->setValue("VHLT_MAX",200);       // Max. HLT volume
    RegEbrew->setValue("VMLT_MAX",110);       // Max. MLT volume
    RegEbrew->setValue("VBOIL_MAX",140);      // Max. Boil kettle volume
    RegEbrew->setValue("VHLT_MIN",40);        // Min. HLT volume needed for electric heating
    RegEbrew->setValue("VBOIL_MIN",40);       // Min. Boil kettle volume needed for electric heating

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
    RegEbrew->setValue("BOIL_MIN_TEMP",60);   // Min. Temp. for Boil-kettle (Celsius)
    RegEbrew->setValue("SP_PREBOIL",95);      // Pre-Boil Temperature (Celsius)
    RegEbrew->setValue("BOIL_DETECT",99.3);   // Boiling-Detection minimum Temperature (Celsius)
    RegEbrew->setValue("SP_BOIL",105);        // Boil Temperature (Celsius)
    RegEbrew->setValue("LIMIT_BOIL",100);     // Limit output during boil (%)
    RegEbrew->setValue("CB_Boil_Rest",1);     // Let wort rest for 5 minutes after boiling
    RegEbrew->setValue("CB_Hop_Alarm",1);     // Sound 4-beeps alarm when hop-gift is needed
    RegEbrew->setValue("CB_BK_recirc",1);     // Start Chilling with recirculating through Boil-kettle
    RegEbrew->setValue("LIMIT_BK_recirc",70); // Limit output during boil (%)

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
    RegEbrew->setValue("THLT_OFFSET"   ,0.0); // Offset for Thlt-I2C sensor
    RegEbrew->setValue("TMLT_OFFSET"   ,0.0); // Offset for Tmlt-I2C sensor
    RegEbrew->setValue("TBOIL_OFFSET"  ,0.0); // Offset for Tboil-OW sensor
    RegEbrew->setValue("TCFC_OFFSET"   ,0.0); // Offset for Tcfc-OW sensor
    RegEbrew->setValue("THLT_OW_OFFSET",0.0); // Offset for Thlt-OW sensor
    RegEbrew->setValue("TMLT_OW_OFFSET",0.0); // Offset for Tmlt-OW sensor
    RegEbrew->setValue("THLT_SENSORS"  ,TSENSOR_USE_I2C); // In case both I2C and OW sensors are present
    RegEbrew->setValue("TMLT_SENSORS"  ,TSENSOR_USE_I2C); // In case both I2C and OW sensors are present
    // Flows
    RegEbrew->setValue("FLOW1_ERR",0);        // Error Correction for FLOW1
    RegEbrew->setValue("FLOW2_ERR",0);        // Error Correction for FLOW2
    RegEbrew->setValue("FLOW3_ERR",0);        // Error Correction for FLOW3
    RegEbrew->setValue("FLOW4_ERR",0);        // Error Correction for FLOW4
    RegEbrew->setValue("FLOW_TEMP_CORR",1);   // Use Temperature Correction
    RegEbrew->setValue("MIN_FR_MLT_PERC",10); // Min. Flowrate for MLT Empty detection
    RegEbrew->setValue("MIN_FR_BOIL_PERC",2); // Min. Flowrate for Boil-kettle Empty detection
    RegEbrew->setValue("VALVE_DELAY",0.3);    // Valve delay/dead-time when closing valve V2 in L
} // MainEbrew::createRegistry()

/*------------------------------------------------------------------
  Purpose  : This function sets the kettle names using the volumes
             found in the Registry. It also sets the minimum
             required volume for enabling electrical heating.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::setKettleVolumes(void)
{
    Vhlt = RegEbrew->value("VHLT_MIN").toReal(); // init. Vhlt
    hlt->setNameVolume(QString("HLT %1 L").arg(RegEbrew->value("VHLT_MAX").toInt()),Vhlt);
    mlt->setNameVolume(QString("MLT %1 L").arg(RegEbrew->value("VMLT_MAX").toInt()),0.0);
    Vboil = RegEbrew->value("VBOIL_MIN").toReal(); // init. Vboil
    boil->setNameVolume(QString("BOIL %1 L").arg(RegEbrew->value("VBOIL_MAX").toInt()),Vboil);
} // MainEbrew::SetKettleVolumes()

/*------------------------------------------------------------------
  Purpose  : This function splits the Registry IP-address and por
             into two variables.
  Variables: ipAddress: a QString holding the IP-address
             ipPort   : an integer holding the port number
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::splitIpAddressPort(void)
{
    QString     string = RegEbrew->value("UDP_IP_PORT").toString();
    QStringList list1  = string.split(':');
    if (list1.size() != 2)
    {
        qDebug() << "Error converting IP-address and port from UDP_IP_PORT";
    }
    else
    {
        ipAddress = QHostAddress(list1.at(0));
        QString s = list1.at(1);
        ipPort    = s.toInt();
    } // else
} // MainEbrew::splitIpAddressPort()

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

       line = in.readLine(); // Read initial HLT water volume
       list1 = line.split(':');
       if (list1.size() >= 2)
            Vhlt_init = list1.at(1).toInt();
       else Vhlt_init = 0.0; // error in maisch.sch
       double vhltmax = RegEbrew->value("VHLT_MAX").toDouble();
       if (Vhlt_init > vhltmax) Vhlt_init = vhltmax;

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
       int done = false;
       while ((ms_tot < MAX_MS) && !in.atEnd() && !done)
       {  // read line
          line  = in.readLine();
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
          else done = true; // empty line, end of temp. time pairs
       } // while
       statusMashScheme->setText(sbar);

       sbar.clear(); // clear QString for statusbar
       sbar.append("Hops: ");
       i    = 0;
       while ((i++ < 2) && !in.atEnd())
       {  // read 2 dummy lines for hop-gift explanations
          line = in.readLine();
       } // while
       done = false;
       hopTimes.clear(); // clear hop-times list
       hopTexts.clear(); // clear hop descriptions
       while (!in.atEnd() && !done)
       {
           line  = in.readLine();
           list1 = line.split(',');
           if (list1.size() >= 2)
           {
               QString s = list1.at(0);
               hopTimes.append(s.toInt()); // hop-time in minutes
               hopTexts << list1.at(1);    // hop description
               sbar.append(s);
               sbar.append(" min. ");
           } // if
           else done = true; // empty line, end of hop-gift times
       } // while
       for (i = 0; i < MAX_HOPS; i++) hopCb[i] = false; // init checkboxes
       if (sbar.size() < 8) sbar.append("-");
       statusHops->setText(sbar);
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
            if ((i < ms_tot - 1) && (ms[i+1].temp > ms[i].temp))
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
        case     Qt::Key_M: switch (ebrew_std)
                            {
                                case S01_WAIT_FOR_HLT_TEMP:
                                     toolMaltAdded->setChecked(true);
                                     break;
                                case S19_RDY_TO_ADD_MALT:
                                     toolStartAddMalt->setChecked(true);
                                     break;
                                case S15_ADD_MALT_TO_MLT:
                                     toolMaltAdded->setChecked(true);
                                     break;
                                case S10_WAIT_FOR_BOIL:
                                     toolBoilStarted->setChecked(true);
                                     break;
                                case S12_BOILING_FINISHED:
                                     toolStartChilling->setChecked(true);
                                     break;
                                case S34_CHILL_BK_READY:
                                     toolStartChilling->setChecked(true);
                                     break;
                                case S16_CHILL_PUMP_FERMENTOR:
                                     toolReadyChilling->setChecked(true);
                                     break;
                            } // switch
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

    mlt->setValues(tmlt,tset_mlt,Vmlt,0.0);
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

    //------------------------------------------------------------------
    // Update the Auto/Manual MessageBox
    //------------------------------------------------------------------
    if ((V1->getStatus() == MANUAL_OFF) || (V1->getStatus() == MANUAL_ON) ||
        (V2->getStatus() == MANUAL_OFF) || (V2->getStatus() == MANUAL_ON) ||
        (V3->getStatus() == MANUAL_OFF) || (V3->getStatus() == MANUAL_ON) ||
        (V4->getStatus() == MANUAL_OFF) || (V4->getStatus() == MANUAL_ON) ||
        (V6->getStatus() == MANUAL_OFF) || (V6->getStatus() == MANUAL_ON) ||
        (V7->getStatus() == MANUAL_OFF) || (V7->getStatus() == MANUAL_ON) ||
        (P1->getStatus() == MANUAL_OFF) || (P1->getStatus() == MANUAL_ON) ||
        (P2->getStatus() == MANUAL_OFF) || (P2->getStatus() == MANUAL_ON))
    {
        autoManualText->setTextColor(Qt::red);
        autoManualText->setText("Manual Override Active!");
        autoManualText->setSubText("At least 1 Pump or Valve is set to Manual Mode");
    } // if
    else if (tset_hlt_sw || tset_boil_sw || gamma_hlt_sw || gamma_boil_sw || thlt_sw ||
             tmlt_sw     || tboil_sw     || vhlt_sw      || vmlt_sw       || vboil_sw)
    {
        autoManualText->setTextColor(Qt::red);
        autoManualText->setText("Switch/Fix Active");
        autoManualText->setSubText("Press Cancel in the Fix Parameters Dialog to clear");
    } // else if
    else
    {
        autoManualText->setTextColor(Qt::green);
        autoManualText->setText("Auto-All");
        autoManualText->setSubText("All Pumps/Valves are in Auto mode, no switches set");
    } // else
    schedulerEbrew->updateDuration("aliveLed",timer.nsecsElapsed()/1000);
} // MainEbrew::task_alive_led()

/*------------------------------------------------------------------
  Purpose    : TASK: Ebrew State Transition Diagram (STD).
  Period-Time: 1.0 second
  ------------------------------------------------------------------*/
void MainEbrew::task_update_std(void)
{
    QElapsedTimer timer;       // time measurements for task
    QString       string;      // temp. string for Ebrew HW comm.
    QColor        color;       // color for pipes
    uint16_t      std_out;     // values of valves
    uint8_t       pump_bits;   // values of pumps

    timer.start(); // Task time-measurement
    std_out = stateMachine(); // call the Ebrew STD
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
    pipeH10->setColor(color);
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
  Period-Time: 1 seconds. Since the Registry TS parameter is in seconds and
               this task is called every second, TS can be used here directly.
  ---------------------------------------------------------------------------*/
void MainEbrew::task_pid_control(void)
{
    QElapsedTimer timer;
    QString       string;

    timer.start(); // Task time-measurement
    if (++pidCntr >= RegEbrew->value("TS").toInt())
    {   // call PID-controllers every TS seconds
        pidCntr = 0;                                          // reset counter
        PidCtrlHlt->pidEnable(hltPid->getButtonState());      // PID is enabled if PowerButton state is ON
        gamma_hlt = PidCtrlHlt->pidControl(thlt,tset_hlt);    // run pid-controller for HLT
        if (PidCtrlBk->pidGetStatus() != PID_FFC)             // PID_FFC is set by the STD
            PidCtrlBk->pidEnable(boilPid->getButtonState());  // PID is enabled if PowerButton state is ON
        gamma_boil = PidCtrlBk->pidControl(tboil,tset_boil);  // run pid-controller for Boil-kettle
    } // if

    // Run switches/fixes and sending to Ebrew HW every second
    if (gamma_hlt_sw) gamma_hlt = gamma_hlt_fx;

    //---------------------
    // HLT Energy-sources
    //---------------------
    uint8_t ena = hlt->getHeatingOptions(); // get all enabled HLT heating-sources
    // Hysteresis for non-modulating gas-burner
    if      (gamma_hlt > RegEbrew->value("GAS_NON_MOD_HLIMIT").toInt()) hltGasNonMod = true;
    else if (gamma_hlt < RegEbrew->value("GAS_NON_MOD_LLIMIT").toInt()) hltGasNonMod = false;
    // Hysteresis for modulating gas-burner
    if (toolHLTPilotLight->isChecked())
    {   // Pilot-light checkbox in menu-bar
        hltGasMod = true;
    } // if
    else
    {   // Checkbox has priority over hysteresis
        if      (gamma_hlt > RegEbrew->value("GAS_MOD_PWM_HLIMIT").toInt()) hltGasMod = true;
        else if (gamma_hlt < RegEbrew->value("GAS_MOD_PWM_LLIMIT").toInt()) hltGasMod = false;
    } // else
    if ((ena & GAS_MODULATING)     && !hltGasMod)    ena &= ~GAS_MODULATING;
    if ((ena & GAS_NON_MODULATING) && !hltGasNonMod) ena &= ~GAS_NON_MODULATING;
    if ((ena & (ELECTRIC_HEATING1 | ELECTRIC_HEATING2 | ELECTRIC_HEATING3)) && triacTooHot)
        ena &= ~(ELECTRIC_HEATING1 | ELECTRIC_HEATING2 | ELECTRIC_HEATING3);
    string = QString("H%1 %2").arg(ena).arg(gamma_hlt,1,'f',0); // PID-Output [0%..100%]
    //qDebug() << "H" << ena << " " << gamma_hlt;
    commPortWrite(string.toUtf8()); // Send it, even if all energy-sources are disabled

    //----------------------------
    // Boil-Kettle Energy-sources
    //----------------------------
    ena = boil->getHeatingOptions(); // get all enabled boil-kettle heating-sources
    if ((ebrew_std == S11_BOILING) && (gamma_boil > RegEbrew->value("LIMIT_BOIL").toDouble()))
    {   // Limit Boil-kettle output during boiling
        gamma_boil = RegEbrew->value("LIMIT_BOIL").toDouble();
    } // if
    if (gamma_boil_sw) gamma_boil = gamma_boil_fx;
    // Hysteresis for non-modulating gas-burner
    if      (gamma_boil > RegEbrew->value("GAS_NON_MOD_HLIMIT").toInt()) boilGasNonMod = true;
    else if (gamma_boil < RegEbrew->value("GAS_NON_MOD_LLIMIT").toInt()) boilGasNonMod = false;
    // Hysteresis for modulating gas-burner
    if      (gamma_boil > RegEbrew->value("GAS_MOD_PWM_HLIMIT").toInt()) boilGasMod = true;
    else if (gamma_boil < RegEbrew->value("GAS_MOD_PWM_LLIMIT").toInt()) boilGasMod = false;
    if ((ena & GAS_MODULATING)     && !boilGasMod)    ena &= ~GAS_MODULATING;
    if ((ena & GAS_NON_MODULATING) && !boilGasNonMod) ena &= ~GAS_NON_MODULATING;
    if ((ena & (ELECTRIC_HEATING1 | ELECTRIC_HEATING2 | ELECTRIC_HEATING3)) && triacTooHot)
        ena &= ~(ELECTRIC_HEATING1 | ELECTRIC_HEATING2 | ELECTRIC_HEATING3);
    string = QString("B%1 %2").arg(ena).arg(gamma_boil,1,'f',0); // PID-Output [0%..100%]
    //qDebug() << "B" << ena << " " << gamma_boil;
    commPortWrite(string.toUtf8()); // Send it, even if all energy-sources are disabled
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
    if (ReadDataAvailable && (ReadData.indexOf("T=") != -1) && (ReadData.size() >= 38))
    {
        QByteArrayList list = ReadData.split(','); // split array in sub-arrays
        if (list.size() >= 7)
        {   // at least 7 temperature values
            QByteArray ba = list.at(0);
            ba.remove(0,2); // remove "T=" in 1st byte-array
            ttriac   = ba.toDouble();
            thlt_i2c = list.at(1).toDouble();
            tmlt_i2c = list.at(2).toDouble();
            tboil    = list.at(3).toDouble();
            tcfc     = list.at(4).toDouble();
            thlt_ow  = list.at(5).toDouble();
            tmlt_ow  = list.at(6).toDouble();
        } // if
    } // if
    else qDebug() << "task_read_temps() error: " << ReadData; // error

    //------------------ TEMP1 (LM35) -----------------------------------------
    if (ttriac_sw)
    {  // Switch & Fix
       ttriac = ttriac_fx;
    } // if
    //------------------------------------------------------
    // SSR/Triac Temperature Protection: hysteresis function
    //------------------------------------------------------
    if      (ttriac >= RegEbrew->value("TTRIAC_HLIM").toInt()) triacTooHot = true;
    else if (ttriac <  RegEbrew->value("TTRIAC_LLIM").toInt()) triacTooHot = false;

    //------------------ TEMP2 (THLT-I2C) --------------------------------------
    if (thlt_i2c > SENSOR_VAL_LIM_OK)
    {
         thlt_i2c += RegEbrew->value("THLT_OFFSET").toDouble(); // update THLT with calibration value
         sensorAlarmInfo &= ~SENS_THLT_I2C; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_THLT_I2C;  // set alarm
    //------------------ TEMP3 (TMLT-I2C) --------------------------------------
    if (tmlt_i2c > SENSOR_VAL_LIM_OK)
    {
         tmlt_i2c += RegEbrew->value("TMLT_OFFSET").toDouble(); // update TMLT with calibration value
         sensorAlarmInfo &= ~SENS_TMLT_I2C; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_TMLT_I2C;  // set alarm
    //------------------ TEMP4 (TBOIL) ----------------------------------------
    if (tboil > SENSOR_VAL_LIM_OK)
    {
         tboil += RegEbrew->value("TBOIL_OFFSET").toDouble(); // update TBOIL with calibration value
         sensorAlarmInfo &= ~SENS_TBOIL; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_TBOIL;  // set alarm
    if (tboil_sw)
    {  // Switch & Fix
       tboil = tboil_fx;
    } // if
    //------------------ TEMP5 (TCFC) -----------------------------------------
    if (tcfc > SENSOR_VAL_LIM_OK)
    {
         tcfc += RegEbrew->value("TCFC_OFFSET").toDouble(); // update TCFC with calibration value
         sensorAlarmInfo &= ~SENS_TCFC; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_TCFC;  // set alarm
    // No switch/fix needed for TCFC
    //------------------ TEMP6 (THLT-OW) ---------------------------------------
    if (thlt_ow > SENSOR_VAL_LIM_OK)
    {
         thlt_ow += RegEbrew->value("THLT_OW_OFFSET").toDouble(); // update THLT-OW with calibration value
         sensorAlarmInfo &= ~SENS_THLT_OW; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_THLT_OW;  // set alarm

    //------------------ TEMP7 (TMLT-OW) ---------------------------------------
    if (tmlt_ow > SENSOR_VAL_LIM_OK)
    {
         tmlt_ow += RegEbrew->value("TMLT_OW_OFFSET").toDouble(); // update TMLT-OW with calibration value
         sensorAlarmInfo &= ~SENS_TMLT_OW; // reset bit in sensor_alarm
    } // if
    else sensorAlarmInfo |= SENS_TMLT_OW;  // set alarm

    // ----------------- Now process thlt_i2c and thlt_ow to form thlt ---------
    int x = RegEbrew->value("THLT_SENSORS").toInt();
    switch (sensorAlarmInfo & SENS_THLTS)
    {
        case SENS_THLT_I2C:   // I2C sensor is not present, OW sensor is present
             thlt = thlt_ow;  // use OW sensor for thlt
             T4->hide();      // Hide temp4 object
             break;
        case SENS_THLT_OW:    // OW sensor is not present, I2C sensor is present
             thlt = thlt_i2c; // use I2C sensor for thlt
             T4->hide();      // Hide temp4 object
             break;
        case 0x0000:          // both OW and I2C sensors are present
             if (x == TSENSOR_AVERAGING)
             {
                  thlt = 0.5*(thlt_i2c + thlt_ow);
                  T4->hide(); // Hide temp4 object
             } // if
             else if (x == TSENSOR_USE_I2C)
             {
                  thlt = thlt_i2c;           // thlt_ow can be used for another temperature measurement
                  T4->show();                // Show temp4 object
                  T4->setTempValue(thlt_ow); // show this temperature measurement
                  T4->update();
             } // else if
             else
             {   // x == TSENSOR_USE_OW
                 thlt = thlt_ow;  // TSENSOR_USE_OW, thlt_i2c is not used
                 T4->hide();      // Hide temp4 object
             } // else
             break;
        default:                       // both OW and I2C sensors are NOT present
             thlt = SENSOR_VAL_LIM_OK; // indicate error
             T4->hide();               // Hide temp4 object
             break;                    // handle in generic alarm part, see below
    } // switch
    if (thlt_sw)
    {  // Switch & Fix
       thlt = thlt_fx;
    } // if
    // ----------------- Now process tmlt_i2c and tmlt_ow to form tmlt ---------
    x = RegEbrew->value("TMLT_SENSORS").toInt();
    switch (sensorAlarmInfo & SENS_TMLTS)
    {
        case SENS_TMLT_I2C:   // I2C sensor is not present, OW sensor is present
             tmlt = tmlt_ow;  // use OW sensor for tmlt
             T5->hide();      // Hide temp5 object
             break;
        case SENS_TMLT_OW:    // OW sensor is not present, I2C sensor is present
             tmlt = tmlt_i2c; // use I2C sensor for tmlt
             T5->hide();      // Hide temp5 object
             break;
        case 0x0000:          // both OW and I2C sensors are present
             if (x == TSENSOR_AVERAGING)
             {
                  tmlt = 0.5*(tmlt_i2c + tmlt_ow);
                  T5->hide(); // Hide temp5 object
             } // if
             else if (x == TSENSOR_USE_I2C)
             {
                  tmlt = tmlt_i2c;           // tmlt_ow is now another temperature
                  T5->show();                // Show temp5 object
                  T5->setTempValue(tmlt_ow); // show this temperature measurement
                  T5->update();
             } // else if
             else
             {   // x = TSENSOR_USE_OW
                 tmlt = tmlt_ow;  // TSENSOR_USE_OW, tmlt_i2c is not used
                 T5->hide();      // Hide temp5 object
             } // else
             break;
        default:                       // both OW and I2C sensors are NOT present
             tmlt = SENSOR_VAL_LIM_OK; // indicate error
             T5->hide();               // Hide temp5 object
             break;                    // handle in generic alarm part, see below
    } // switch
    if (tmlt_sw)
    {  // Switch & Fix
       tmlt = tmlt_fx;
    } // if
    // Now test is one or more required sensors have their alarm bit set
    if (((alarmSound == ALARM_TEMP_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS)) &&
        (((sensorAlarmInfo & SENS_THLTS) == SENS_THLTS) || /* Both HLT temp. sensors are NOT present */
         ((sensorAlarmInfo & SENS_TMLTS) == SENS_TMLTS) || /* Both MLT temp. sensors are NOT present */
          (sensorAlarmInfo & (SENS_TBOIL | SENS_TCFC))))   /* The Boil-kettle or the CFC-output sensors are NOT present */
    {
         commPortWrite("X3"); // sound the alarm
    } // if
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
    FlowCfcOut -= FlowCfcOutResetValue; // reset flow4 at end of boiling
    F3->setFlowValue(FlowCfcOut ,tcfc);
    F4->setFlowValue(Flow4      ,thlt);

    //------------------ FLOW1 ------------------------------------------------
    if ((F1->getFlowRate(FLOWRATE_RAW) < 0.1) && flow1Running)
    {
         sensorAlarmInfo |=  SENS_FLOW1;
         F1->setError(true);
         if ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
             commPortWrite("X2");
         } // if
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW1;
        F1->setError(false);
    } // else
    Vhlt = Vhlt_init - F1->getFlowValue(); // adjust actual volume in HLT
    if (vhlt_sw)
    {  // Switch & Fix
       Vhlt = vhlt_fx;
    } // if

    //------------------ FLOW2 ------------------------------------------------
    if ((F2->getFlowRate(FLOWRATE_RAW) < 0.1) && flow2Running)
    {
         sensorAlarmInfo |=  SENS_FLOW2;
         F2->setError(true);
         if ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
             commPortWrite("X2"); // sound alarm
         } // if
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW2;
        F2->setError(false);
    } // else
    Vmlt = F1->getFlowValue() - F2->getFlowValue(); //use these values instead of FlowHltMlt - FlowMltBoil
    if (vmlt_sw)
    {  // Switch & Fix
       Vmlt = vmlt_fx;
    } // if

    //------------------ FLOW3 ------------------------------------------------
    if ((F3->getFlowRate(FLOWRATE_RAW) < 0.1) && flow3Running)
    {
         sensorAlarmInfo |=  SENS_FLOW3;
         F3->setError(true);
         if ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
             commPortWrite("X2"); // sound alarm
         } // if
    } // if
    else
    {
        sensorAlarmInfo &= ~SENS_FLOW3;
        F3->setError(false);
    } // else
    Vboil = F2->getFlowValue() - F3->getFlowValue(); // use these values instead of FlowMltBoil - FlowCfcOut
    if (vboil_sw)
    {  // Switch & Fix
       Vboil = vboil_fx;
    } // if

    //------------------ FLOW4 ------------------------------------------------
    if ((F4->getFlowRate(FLOWRATE_RAW) < 0.1) && flow4Running)
    {
         sensorAlarmInfo |=  SENS_FLOW4;
         F4->setError(true);
         if ((alarmSound == ALARM_FLOW_SENSORS) || (alarmSound == ALARM_TEMP_FLOW_SENSORS))
         {
             commPortWrite("X2"); // sound alarm
         } // if
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
   QMessageBox::about(this,QString("About Ebrew 3.0 Qt r%1").arg(ebrewRevision.mid(11,4)),
                QString("This program is used to fully control a HERMS home-brewing system. "
                        "It communicates with the <b>Ebrew hardware</b>, which is a"
                        " dedicated hardware solution, based around an <b>Arduino Nano</b> with a <b>Wiz550io</b> Ethernet Controller.<br><br>"

                        "This version is a redesign from Ebrew 2.0, that was created"
                        " with Borland C++ Builder and was in use from 2003 - 2020. It's latest revision was r1.99.<br><br>"

                        "This version (Ebrew 3.0 Qt r%1) is a complete redesign from that Borland version and is now built with Qt 5.14.<br><br>"

                        "Web-site: <a href=\"www.vandelogt.nl\">www.vandelogt.nl</a><br>"
                        "Github: <a href=\"https://github.com/Emile666/Ebrew_Qt\">https://github.com/Emile666/Ebrew_Qt</a><br>"
                        "Brewery: Brouwerij de Boezem, The Netherlands").arg(ebrewRevision.mid(11,4)));
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
             Edit->Terminal Editor is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuEditTerminal(void)
{
    auto Dialog = new DialogEditTerminal(this);

    Dialog->show();
} // MainEbrew::MenuEditTerminal()

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
             View->Status & Alarms is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void MainEbrew::MenuViewStatusAlarms(void)
{
    auto Dialog = new DialogViewStatusAlarms(this);

    Dialog->show();
} // MainEbrew::MenuViewStatusAlarms()

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
    QUdpSocket  *u      = new QUdpSocket(this);

    serialPort    = p; // copy pointer to MainEbrew
    udpSocket     = u; // copy pointer to MainEbrew
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
        if (u->bind(ipPort))
        {
            comPortIsOpen = true;
            connect(u, &QUdpSocket::readyRead,this,&MainEbrew::commPortRead);
        } // if
        else
        {
            comPortIsOpen = false;
            qDebug() << "UDP-bind error";
        } // else
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
        } // if
    } // if
    else
    {   // Ethernet socket
        if (comPortIsOpen)
        {
            udpSocket->flush();
            udpSocket->close();
        } // if
    } // else
    comPortIsOpen     = false;
    ReadDataAvailable = false;
    ReadData.clear();

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
        if (comPortIsOpen)
        {
            ReadDataAvailable = false;
            ReadData.clear();
            int x = udpSocket->writeDatagram(s+"\n",ipAddress,8888);
            if (x == -1) qDebug() << "UDP write error";
        }   // if
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
             For the Ethernet connection, this routine is also used in a
             signal-slot combination. Signal is readyRead() from the
             UdpSocket, slot is this function.
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
        ReadData.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(ReadData.data(), ReadData.size(),&ebrewHwIp);
        removeLF(ReadData); // remove \n
        if (ReadData.size() > 0) ReadDataAvailable = true;
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
uint16_t MainEbrew::stateMachine(void)
{
    //----------------------------------------------------------------------------
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
    //----------------------------------------------------------------------------
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
    // State 14: 0  0  0  0  0  0  1  0  1  1  Pump Prefill              0x000B
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
    // State 33: 0  1  0  0  1  0  0  1  0  0  Chill wort in Boil-kettle 0x0124
    // State 34: 0  0  0  0  0  0  0  0  0  0  Boil-kettle chill ready   0x0000
    // State 35: 0  1  0  0  1  0  0  1  0  0  Sanitize Chiller          0x0124
    // State 36: 1  0  0  0  0  0  0  0  1  0  Grainfather Heater only   0x0202
    //----------------------------------------------------------------------------
    uint16_t  actuatorSettings[STD_MAX+1] =
                           /* 00 */{0x0000, 0x0200, 0x030B, 0x0309, 0x0309,  /* 04 */
                           /* 05 */ 0x0309, 0x0141, 0x030B, 0x0000, 0x0141,  /* 09 */
                           /* 10 */ 0x0000, 0x0000, 0x0000, 0x0200, 0x000B,  /* 14 */
                           /* 15 */ 0x0000, 0x0124, 0x0000, 0x0000, 0x0309,  /* 19 */
                           /* 20 */ 0x0000, 0x016C, 0x016C, 0x0000, 0x012C,  /* 24 */
                           /* 25 */ 0x0124, 0x0000, 0x0142, 0x0122, 0x010A,  /* 29 */
                           /* 30 */ 0x0006, 0x0003, 0x0000, 0x0124, 0x0000,  /* 34 */
                           /* 35 */ 0x0124, 0x0202};

    bool      maltAdded; // help var. in state S01_WAIT_FOR_HLT_TEMP
    QString   string;    // For stdText->setText()
    QString   substring; // For stdText->setSubText()

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
            tset_boil = TEMP_DEFAULT;            // Setpoint Temp. for Boil-kettle
            if (toolStartCIP->isChecked())  // Is Start CIP checkbox checked at top toolbar?
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
            } // if
            else if (toolGFSpargeWater->isChecked())
            {
                hltPid->setButtonState(true);  // Enable PID-controller for HLT
                ebrew_std = S36_GF_HEATER_ONLY;
            } // else if
            else if (hltPid->getButtonState())        // Is PowerButton pressed for HLT PID controller?
            {  // start with normal brewing states
                ebrew_std = S01_WAIT_FOR_HLT_TEMP;
                burner_on << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
            } // if
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
            if (RegEbrew->value("CB_Malt_First").toInt() > 0)
            {   // Enable 'Malt is added' checkbox at top-toolbar
                toolMaltAdded->setEnabled(true);
            } // if
            tset_mlt  = ms[ms_idx].temp;         // get temp. from mash-scheme
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_boil = TEMP_DEFAULT;            // Setpoint Temp. for Boil-kettle
            maltAdded = (RegEbrew->value("CB_Malt_First").toInt() == 0) || toolMaltAdded->isChecked();
            if ((thlt >= tset_hlt) && maltAdded && !toolGFSpargeWater->isChecked())
            {   // HLT TEMP is OK and malt is added when MaltFirst option is selected
                toolStartCIP->setEnabled(false);     // Hide CIP option at toolbar
                if (toolMaltAdded->isChecked()) toolMaltAdded->setEnabled(false); // disable checkbox, no longer needed
                toolGFSpargeWater->setEnabled(false); // Hide this option from now on
                Vhlt_old  = Vhlt; // remember old value
                timer3    = 0;    // init. '1 minute' timer
                ebrew_std = S14_PUMP_PREFILL;
            } // if
            else if (toolStartCIP->isChecked())  // Is Start CIP checkbox checked at top toolbar?
            {
                ebrew_std = S00_INITIALISATION;  // Back to init. state and then to CIP-states
            } // else if
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
            tset_boil = TEMP_DEFAULT;            // Setpoint Temp. for Boil-kettle
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
            tset_boil = TEMP_DEFAULT;            // Setpoint Temp. for Boil-kettle
            if (Vmlt >= mash_vol - RegEbrew->value("VALVE_DELAY").toDouble())
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
            if (ms_idx == 0)
            {   // make sure that tset_hlt is updated if TOffset0 is changed in this state
                tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            }
            else
            {   // this is the value from S13_MASH_PREHEAT_HLT, same here if TOffset is changed
                tset_hlt  = ms[ms_idx].temp + 2 * RegEbrew->value("TOffset").toDouble();
            } // else
            tset_mlt  = ms[ms_idx].temp; // get temp. from mash-scheme
            // tset_hlt is NOT set here, but in previous state (FILL_MLT or MASH_PREHEAT_HLT)
            tset_boil = TEMP_DEFAULT;    // Setpoint Temp. for Boil-kettle
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
                    ms[ms_idx].timer      = 0; // start the corresponding mash timer
                    ms[ms_idx].time_stamp = QTime::currentTime().toString(); // save time-stamp for Progress Dialog
                    ebrew_std             = S04_MASH_TIMER_RUNNING;
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
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_mlt  = ms[ms_idx].temp; // get temp. from mash-scheme
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
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset0").toDouble(); // compensate for dough-in losses
            tset_mlt  = ms[ms_idx].temp; // get temp. from mash-scheme
            toolStartAddMalt->setEnabled(false); // disable checkbox, no longer needed
            toolMaltAdded->setEnabled(true);     // enable checkbox 'Malt added to MLT'
            if (toolMaltAdded->isChecked())
            {  // malt is added to MLT, start mash timer
               ms[ms_idx].timer      = 0; // start the corresponding mash timer
               ms[ms_idx].time_stamp = QTime::currentTime().toString(); // save time-stamp for Progress Dialog
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
            ms[ms_idx].timer++; // increment mash timer
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble();  // Single offset
            tset_boil = TEMP_DEFAULT; // Setpoint Temp. for Boil-kettle
            if (ms_idx < ms_tot - 1)
            {  // There's a next mash phase
                substring = QString("After timeout in %1 min., HLT is heated to the next temperature").arg((ms[ms_idx].preht - ms[ms_idx].timer)/60);
                if (ms[ms_idx].timer >= ms[ms_idx].preht)
                {
                    ebrew_std = S13_MASH_PREHEAT_HLT;
                } // if
                // else remain in this state (timer is incremented)
            } // if
            else
            {  // This is the last mash phase, continue with sparging
                substring = QString("After timeout, mashing is finished and sparging will start");
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
            tset_boil = TEMP_DEFAULT; // Setpoint Temp. for Boil-kettle
            if (ms[ms_idx].timer >= ms[ms_idx].time) // time-out?
            {
                ms_idx++; // increment index in mash scheme
                updateMsIdxStatusBar(); // update ms_idx in Statusbar
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
            tset_boil = TEMP_DEFAULT; // Setpoint Temp. for Boil-kettle
            if (ms_idx < ms_tot - 1)
            {  // There's a next mash phase
                if ((ms[ms_idx].preht > 0) && (ms[ms_idx].timer >= ms[ms_idx].preht))
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
            string    = QString("05. Sparge-timer Running (%1/%2 min., batch %3/%4)").arg(timer1/60).arg(RegEbrew->value("SP_TIME").toInt()).arg(sp_idx).arg(RegEbrew->value("SP_BATCHES").toInt());
            if (sp_idx < RegEbrew->value("SP_BATCHES").toInt())
                 substring = QString("After timeout, wort is pumped from the MLT to the Boil-kettle");
            else substring = QString("After timeout, sparging is finished and the MLT is emptied");
            tset_mlt = ms[ms_idx].temp;
            tset_hlt = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boilPid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boilPid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = TEMP_DEFAULT;
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
            string    = QString("06. Pump from MLT to Boil-kettle (%1 L, batch %2/%3)").arg(sp_idx ? sp_vol_batch : sp_vol_batch0,2,'f',1).arg(sp_idx).arg(RegEbrew->value("SP_BATCHES").toInt());
            substring = QString("Wort is pumped to the Boil-kettle");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boilPid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boilPid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = TEMP_DEFAULT;

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
            string    = QString("07. Pump fresh water from HLT to MLT (%1 L, batch %2/%3)").arg(sp_vol_batch,2,'f',1).arg(sp_idx).arg(RegEbrew->value("SP_BATCHES").toInt());
            substring = QString("A batch of fresh sparge water is added from the HLT to the MLT");
            tset_mlt  = ms[ms_idx].temp;
            tset_hlt  = tset_mlt + RegEbrew->value("TOffset").toDouble(); // Single offset
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boilPid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boilPid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = TEMP_DEFAULT;

            if (Vmlt >= Vmlt_old + sp_vol_batch - RegEbrew->value("VALVE_DELAY").toDouble())
            {
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
            if ((tboil > RegEbrew->value("BOIL_MIN_TEMP").toDouble()) || boilPid->getButtonState())
            {  // There is sufficient wort in the Boil-kettle
                tset_boil = RegEbrew->value("SP_PREBOIL").toDouble(); // PreBoil Temperature Setpoint
                boilPid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            } // if
            else tset_boil = TEMP_DEFAULT;

            if (++timer2 >= TMR_DELAY_xSEC)
            {
                hlt2mlt << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                sp_idx++;               // Increase #Sparging Sessions
                updateSpIdxStatusBar(); // update sp_idx in Statusbar
                Vhlt_old  = Vhlt;       // remember old value
                Vmlt_old  = Vmlt;       // remember current MLT volume
                timer2    = 0;          // reset timer2
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
            tset_hlt  = TEMP_DEFAULT;       // Disable HLT PID-Controller
            hltPid->setButtonState(false);  // Disable PID-controller for HLT
            tset_boil = RegEbrew->value("SP_BOIL").toDouble();  // Boil Temperature Setpoint
            boilPid->setButtonState(true);  // Enable PID-Controller for Boil-kettle
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
            tset_hlt  = TEMP_DEFAULT;          // disable heating element
            tset_boil = RegEbrew->value("SP_BOIL").toDouble(); // Boil Temperature Setpoint
            toolBoilStarted->setEnabled(true); // Enable checkbox at top-toolbar
            boilPid->setButtonState(true);     // Enable PID-Controller for Boil-kettle
            if (toolBoilStarted->isChecked() || (tboil > RegEbrew->value("BOIL_DETECT").toDouble()))
            {
                toolBoilStarted->setChecked(true);       // Set checkbox to checked
                toolBoilStarted->setEnabled(false);      // ... and disable again, no longer needed
                Boil << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                timer5    = 0;                           // init. timer for boiling time
                hopIdx    = 0;                           // init. hop-index
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
            boilPid->setButtonState(true); // Enable PID-Controller for Boil-kettle
            if (++timer5 >= boil_time_ticks)
            {
                toolStartChilling->setEnabled(true);     // Enable checkbox at top-toolbar
                Boil << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                brest_tmr = 0; // init boil-rest timer
                ebrew_std = S12_BOILING_FINISHED;
            } // if
            else if ((hopIdx < hopTimes.size()) && !hopCb[hopIdx] &&
                     (hopTimes.at(hopIdx) >= (boil_time_ticks - timer5)/60))
            {
                cbHops.setChecked(false);
                msgBox(QString("Add hop-gift %1 to Boil-kettle at %2 minutes remaining").arg(hopIdx+1).arg(hopTimes.at(hopIdx)),hopTexts.at(hopIdx),&cbHops);
                if (cbHops.isChecked())
                {
                    hopCb[hopIdx] = true;
                    hopIdx++; // increment hop-index
                    cbHops.setChecked(false);
                } // if
                else if (RegEbrew->value("CB_Hop_Alarm").toInt())
                {
                    commPortWrite("X4"); // 4 beeps indicating hop-addition is needed
                } // if
            } // else if
            break;

        //---------------------------------------------------------------------------
        // S12_BOILING_FINISHED: Boiling has finished, remain here until the user
        // has set up everything for chilling, then goto S15_CHILL_PUMP_FERMENTOR
        // Continu with boiling in this state, so that the brewer can manually control
        // a longer boiling time, to achieve final gravity of the wort.
        //---------------------------------------------------------------------------
        case S12_BOILING_FINISHED:
            tset_boil = TEMP_DEFAULT;        // Boil Temperature Setpoint
            boilPid->setButtonState(false);  // Disable PID-Controller for Boil-kettle
            if (((RegEbrew->value("CB_Boil_Rest").toInt() == 0) || (++brest_tmr > TMR_BOIL_REST_5_MIN)) && toolStartChilling->isChecked())
            {  // Init flow3 (cfc-out) flowrate-low detector
                toolStartChilling->setChecked(false);     // Uncheck checkbox \'CFC Prepared, start Chilling\'
                Chill << QTime::currentTime().toString(); // New transition, copy time-stamp into array of strings
                if (RegEbrew->value("CB_BK_recirc").toInt() == 1)
                {
                     brest_tmr = 0; // reset timer
                     ebrew_std = S35_SANITIZE_CHILLER;   // Chill wort in Boil-kettle through recirculation
                } // if
                else
                {   // start directly with cooling and pumping into fermentor
                    toolStartChilling->setEnabled(false); // Disable checkbox \'CFC Prepared, start Chilling\', no longer needed
                    toolReadyChilling->setEnabled(true);  // Enable checkbox 'Chilling finished'
                    FlowCfcOutResetValue = FlowCfcOut;    // reset Flow_cfc_out to count actual volume in Fermenter
                    F3->initFlowRateDetector(RegEbrew->value("MIN_FR_BOIL_PERC").toInt());
                    ebrew_std = S16_CHILL_PUMP_FERMENTOR; // Chill wort and pump directly to Fermenter
                } // else
            } // if
            else if (RegEbrew->value("CB_Boil_Rest").toInt() == 1)
            {
               if (brest_tmr > TMR_BOIL_REST_5_MIN)
                    string = QString("12. Boil Finished");
               else string = QString("12. Boil Finished, wait %1/%2 min.").arg(brest_tmr/60).arg(TMR_BOIL_REST_5_MIN/60);
               if (!toolStartChilling->isChecked()) string.append(", prepare Chiller (M)");
            } // if
            else
            {
                string = QString("12. Boil Finished, CFC-output in ");
                if (RegEbrew->value("CB_BK_recirc").toInt() == 1)
                     string.append("Boil-kettle");
                else string.append("Fermenter");
                if (!toolStartChilling->isChecked()) string.append(", prepare Chiller (M)");
            } // else
            substring   = QString("Prepare chiller. If ready, click \'CFC Prepared, start Chilling\' at top toolbar");
            break;

        //---------------------------------------------------------------------------
        // S35_SANITIZE_CHILLER: The counterflow chiller is sanitized for 5 minutes
        // by pumping hot wort through it, with no cooling flow.
        //---------------------------------------------------------------------------
        case S35_SANITIZE_CHILLER:
             string    = QString("35. Sanitize CFC, CFC-output in Boil-kettle, Cooling OFF");
             substring = QString("Chiller is sanitized for %1 seconds, leave cooling flow OFF").arg(brest_tmr);
             tset_boil = TEMP_DEFAULT;       // Boil Temperature Setpoint
             boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
             if (++brest_tmr > TMR_SANITIZE_CHILLER)
             {
                 ebrew_std = S33_CHILL_BOIL_KETTLE; // Chill wort in Boil-kettle through recirculation
                 commPortWrite("X5"); // 5 beeps to indicate that cooling flow should be enabled
             } // if
             break;

        //---------------------------------------------------------------------------
        // S33_CHILL_BOIL_KETTLE: The boiled wort is sent through the counterflow
        // chiller and pumped back to the Boil-kettle.
        //---------------------------------------------------------------------------
        case S33_CHILL_BOIL_KETTLE:
            string    = QString("33. Chill wort, CFC-output in Boil-kettle, Cooling ON");
            substring = QString("Boil-kettle wort cooling as long as temperature is more than %1 °C").arg(RegEbrew->value("LIMIT_BK_recirc").toInt());
            if (tboil < RegEbrew->value("LIMIT_BK_recirc").toDouble())
            {   // Boil-kettle temperature decreased below minimum
                toolStartChilling->setChecked(false); // Uncheck checkbox \'CFC Prepared, start Chilling\'
                ebrew_std = S34_CHILL_BK_READY;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S34_CHILL_BK_READY: The partially cooled wort in the Boil-kettle can
        // now be transferred to the Fermenter.
        //---------------------------------------------------------------------------
        case S34_CHILL_BK_READY:
            string    = QString("34. Place CFC-output in Fermenter (M)");
            substring = QString("If CFC-output is in Fermenter, press \'CFC Prepared, start Chilling\' at top toolbar");
            tset_boil = TEMP_DEFAULT;       // Boil Temperature Setpoint
            boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
            if (toolStartChilling->isChecked())
            {   // CFC-output is now placed in fermentation-bin
                toolStartChilling->setEnabled(false); // Disable checkbox \'CFC Prepared, start Chilling\', no longer needed
                toolReadyChilling->setEnabled(true);  // Enable checkbox 'Chilling finished'
                FlowCfcOutResetValue = FlowCfcOut;    // reset Flow_cfc_out to count actual volume in Fermenter
                F3->initFlowRateDetector(RegEbrew->value("MIN_FR_BOIL_PERC").toInt());
                ebrew_std = S16_CHILL_PUMP_FERMENTOR;
            } // if
            break;

        //---------------------------------------------------------------------------
        // S16_CHILL_PUMP_FERMENTOR: The boiled wort is sent through the counterflow
        // chiller and directly into the fermentation bin.
        //---------------------------------------------------------------------------
        case S16_CHILL_PUMP_FERMENTOR:
            string    = QString("16. Chill & Pump to Fermentation Bin (M)");
            substring = QString("If end of chilling is not detected automatically, click \'Chilling is finished\' at top toolbar");
            tset_boil = TEMP_DEFAULT;       // Boil Temperature Setpoint
            boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
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
            tset_boil = TEMP_DEFAULT;       // Boil Temperature Setpoint
            boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
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
            tset_hlt  = TEMP_DEFAULT; // HLT setpoint temperature
            tset_boil = TEMP_DEFAULT; // Boil-kettle setpoint temperature
            cip_circ  = 0;            // Init. CIP circulation counter
            boilPid->setButtonState(false);    // Disable PID-Controller for Boil-kettle
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
             substring = QString("NaOH solution is pumped through while heated to setpoint temperature");
             tset_boil = RegEbrew->value("CIP_SP").toDouble(); // Boil-kettle Temperature Setpoint
             PidCtrlBk->pidEnable(PID_FFC);  // Enable Feed-forward control for Boil-kettle
             boilPid->setButtonState(true);  // Enabled PID-Controller Power-button
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
              PidCtrlBk->pidEnable(PID_ON);     // Enable normal control for Boil-kettle
              boilPid->setButtonState(true);    // Enable PID-Controller Power-button
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
               substring = QString("Pipes are being cleaned with NaOH solution");
               tset_boil = RegEbrew->value("CIP_SP").toDouble(); // Boil-kettle Temperature Setpoint
               boilPid->setButtonState(true);    // Enable PID-Controller for Boil-kettle
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
                substring = QString("NaOH solution is removed from the Boil-kettle");
                tset_boil = TEMP_DEFAULT;       // Boil-kettle Temperature Setpoint
                boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
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
                 substring = QString("NaOH solution is removed, now fill HLT with fresh water");
                 tset_boil = TEMP_DEFAULT;       // Boil-kettle Temperature Setpoint
                 boilPid->setButtonState(false); // Disable PID-Controller for Boil-kettle
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
             string    = QString("27. CIP: Clean Output V7 (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_OUT_TIME").toInt());
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
              string    = QString("28. CIP: Clean Output V6 (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_OUT_TIME").toInt());
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
               string    = QString("29. CIP: Clean Output V4 (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_OUT_TIME").toInt());
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
                string    = QString("30. CIP: Clean Input V3 (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_INP_TIME").toInt());
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
                 string    = QString("31. CIP: Clean Input V1 (%1/%2 sec.)").arg(cip_tmr1).arg(RegEbrew->value("CIP_INP_TIME").toInt());
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

            //---------------------------------------------------------------------------
            // S32_CIP_END: Last state of CIP. Uncheck 'Start Clean-in-Place (CIP) at
            //              toolbar at top of screen to return to Initialisation state.
            //---------------------------------------------------------------------------
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
            // S36_GF_HEATER_ONLY: Use the HLT as sparge water kettle for the Grainfather.
            //                     Since valve V2 is on, the hot water can be obtained
            //                     directly from the manual-valve.
            //---------------------------------------------------------------------------
            case S36_GF_HEATER_ONLY:
                string    = QString("36. Grainfather Sparge Water Heater");
                substring = QString("Uncheck checkbox \'GF Sparge Water Heater\' to return to Init. state");
                tset_hlt  = 78.0;                  // Set sparge-water for Grainfather to 78.0 °C
                if (!toolGFSpargeWater->isChecked())
                {
                   setTopToolBar(TOOLBAR_BREWING); // select normal brewing toolbar at top of screen
                   commPortWrite("R0");            // reset all flows to 0.0 L in Ebrew hardware
                   hltPid->setButtonState(false);  // Disable PID-controller for HLT
                   ebrew_std = S00_INITIALISATION;
                } // if
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

    stdText->setVisible(false);
    stdText->setSubText(substring); // Update STD sub-text
    stdText->setText(string);       // Update STD label
    stdText->setVisible(true);

    //-------------------------------------------------
    // Now calculate the proper settings for the valves
    //-------------------------------------------------
    uint16_t actuators;
    if (ebrew_std > STD_MAX)
         actuators = actuatorSettings[S00_INITIALISATION];
    else actuators = actuatorSettings[ebrew_std];
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
} // MainEbrew::stateMachine()
