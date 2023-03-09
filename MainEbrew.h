/**************************************************************************************
** Filename    : MainEbrew.h
** Author      : Emile
** Purpose     : Header file for MainEbrew.cpp. Contains the MainEbrew class, which contains
**               all logic for creating an automated HERMS brewing system.
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
#ifndef MAIN_EBREW_H
#define MAIN_EBREW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QCheckBox>
#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QUdpSocket>

#include "hmi_objects.h"
#include "controlobjects.h"
#include "scheduler.h"

//------------------------------
// Ebrew system-wide defines
//------------------------------
#define EBREW_REVISION "$Revision: 3.21"                        /* Ebrew SW revision number */
#define COMMDBGFILE    "com_port_dbg.txt"                       /* Default filename for COM port logging */
#define LOGFILE        "ebrewlog.txt"                           /* Default Ebrew log-file name */
#define MASHFILE       "maisch.sch"                             /* Default mash-scheme file */
#define REGKEY         "HKEY_CURRENT_USER\\Software\\ebrew\\V3" /* Registry entry */
#define EBREW_HW_ID    "E-Brew"                                 /* S0-reponse from Ebrew hardware */

//------------------------------
// Defines for Sensor Alarms
//------------------------------
#define ALARM_OFF               (0) /* No sound */
#define ALARM_TEMP_SENSORS      (1) /* Audio alarm only on Temp. sensor error */
#define ALARM_FLOW_SENSORS      (2) /* Audio alarm only on flowsensor error */
#define ALARM_TEMP_FLOW_SENSORS (3) /* Audio alarm on all sensor errors */

//---------------------------------
// Bit-defines for sensorAlarmInfo
//---------------------------------
#define SENS_THLT_I2C (0x0001)   /* HLT temperature */
#define SENS_TMLT_I2C (0x0002)   /* MLT temperature */
#define SENS_TBOIL    (0x0004)   /* Boil-kettle temperature */
#define SENS_TCFC     (0x0008)   /* CFC-output temperature */
#define SENS_FLOW1    (0x0010)   /* HLT output flowmeter */
#define SENS_FLOW2    (0x0020)   /* Boil-kettle input flowmeter */
#define SENS_FLOW3    (0x0040)   /* CFC-output flowmeter */
#define SENS_FLOW4    (0x0080)   /* HLT heat-exchanger -> MLT top return manifold */
#define SENS_THLT_OW  (0x0100)   /* HLT-OW temperature */
#define SENS_TMLT_OW  (0x0200)   /* MLT-OW temperature */
#define SENS_THLTS    (SENS_THLT_I2C | SENS_THLT_OW)
#define SENS_TMLTS    (SENS_TMLT_I2C | SENS_TMLT_OW)

//-----------------------------------------------------------
// Defines for COM Port Communication.
// The longest task on ebrew HW is the one-wire task,
// this task lasts max. 22 msec.
// Set WAIT_READ_TIMEOUT > 22 msec.
//-----------------------------------------------------------
#define MAX_READ_RETRIES      (2)
#define NORMAL_READ_TIMEOUT  (40)
#define LONG_READ_TIMEOUT    (80)

//----------------------------------
// Defines for Scheduler tasks
//----------------------------------
#define TS_FLOWS_MSEC (2000) /* Read flowsensors every 2 sec. */
#define TS_TEMPS_MSEC (2000) /* Read temp. sensors every 2 sec. */
#define TS_PID_MSEC   (1000) /* Call PID task every sec. Note: PID itself is called every TS sec. */
#define TS_STD_MSEC   (1000) /* Call state-machine every second */
#define TS_LED_MSEC    (500) /* Alive LED blinking every 1/2 sec. */
#define TS_WR_LOGFILE (5000) /* Write to logfile every 5 seconds */

//----------------------------------
// Defines for Brew-day Settings
//----------------------------------
#define MAX_MS      (10)     /* Max. number of mash temp-time pairs */
#define MAX_HOPS    (10)     /* Max. number of hop-gifts */
#define NOT_STARTED (-1)     /* Timer not started */

//----------------------------------
// Defines for readMashSchemeFile()
//----------------------------------
#define INIT_TIMERS    (true)
#define NO_INIT_TIMERS (false)

//----------------------------------
// Defines for setTopToolBar()
//----------------------------------
#define TOOLBAR_BREWING (0)
#define TOOLBAR_CIP     (1)

//------------------------------------------------------
// Defines for State Transition Diagram.
// The STD is called every second => 1 tick == 1 second.
//------------------------------------------------------
#define S00_INITIALISATION         (0)
#define S01_WAIT_FOR_HLT_TEMP      (1)
#define S14_PUMP_PREFILL          (14)
#define S02_FILL_MLT               (2)

#define S03_WAIT_FOR_MLT_TEMP      (3)
#define S15_ADD_MALT_TO_MLT       (15)
#define S04_MASH_TIMER_RUNNING     (4)
#define S13_MASH_PREHEAT_HLT      (13)

#define S05_SPARGE_TIMER_RUNNING   (5)
#define S06_PUMP_FROM_MLT_TO_BOIL  (6)
#define S07_PUMP_FROM_HLT_TO_MLT   (7)
#define S08_DELAY_xSEC             (8)

#define S09_EMPTY_MLT              (9) /* Pump all remaining wort from MLT into Boil-kettle */
#define S10_WAIT_FOR_BOIL         (10) /* Wait until wort in Boil-kettle start boiling */
#define S11_BOILING               (11) /* Wort is boiling */
#define S12_BOILING_FINISHED      (12) /* Boiling finished, prepare for wort chilling */
#define S33_CHILL_BOIL_KETTLE     (33) /* Boil-kettle Chilling, CFC-output into top of BK */
#define S34_CHILL_BK_READY        (34) /* Boil-kettle Chilling is finished */
#define S35_SANITIZE_CHILLER      (35) /* Sanitize CFC by pumping hot wort through it for 5 min. */
#define S16_CHILL_PUMP_FERMENTOR  (16) /* Chill and transfer wort from Boil-kettle to CFC */
#define S17_FINISHED              (17) /* All wort is pumped into fermentation-bin, brewing session finished */
#define S18_MASH_REST_5_MIN       (18) /* Wait 5 minutes before pump is switched on */
#define S19_RDY_TO_ADD_MALT       (19) /* Ready to add malt to water inside the MLT */

#define S20_CIP_INIT              (20)
#define S21_CIP_HEAT_UP           (21)
#define S22_CIP_CIRC_5_MIN        (22)
#define S23_CIP_REST_5_MIN        (23)
#define S24_CIP_DRAIN_BOIL1       (24)
#define S25_CIP_DRAIN_BOIL2       (25)
#define S26_CIP_FILL_HLT          (26)
#define S27_CIP_CLEAN_OUTPUT_V7   (27)
#define S28_CIP_CLEAN_OUTPUT_V6   (28)
#define S29_CIP_CLEAN_OUTPUT_V4   (29)
#define S30_CIP_CLEAN_INPUT_V3    (30)
#define S31_CIP_CLEAN_INPUT_V1    (31)
#define S32_CIP_END               (32)

#define S36_GF_HEATER_ONLY        (36)
#define STD_MAX                   (36) /* Max. number for ebrew_std */

//-------------------------------------------------------------
// Hard-coded Timers.
// task update_std() runs every second, so 1 second is 1 tick.
//-------------------------------------------------------------
#define TMR_PREFILL_PUMP       (60) /* Pump priming time in seconds */
#define TMR_DELAY_xSEC         (10) /* Default delay in seconds */
#define TMR_MASH_REST_5_MIN   (300) /* Mash-rest in seconds */
#define TMR_BOIL_REST_5_MIN   (300) /* Post-boil rest in seconds */
#define TMR_SANITIZE_CHILLER  (300) /* Sanitize chiller rest in seconds */

//--------------------------------------------------------------------------
// #defines for the valves. Each valve can be set manually or automatically
// by the STD. Bit-values are for the variable 'valves'.
//--------------------------------------------------------------------------
#define P1b  (0x0200) /* Pump P2, HLT heat-exchanger pump */
#define P0b  (0x0100) /* Pump P1, main Ebrew pump */
#define V8b  (0x0080) /* Valve V8, futures use */
#define V7b  (0x0040) /* Valve V7, Boil-kettle input */
#define V6b  (0x0020) /* Valve V6, CFC-output */
#define V5b  (0x0010) /* Valve V5, future use */
#define V4b  (0x0008) /* Valve V4, HLT heat-exchanger -> MLT top return manifold */
#define V3b  (0x0004) /* Valve V3, Boil-kettle output */
#define V2b  (0x0002) /* Valve V2, HLT output */
#define V1b  (0x0001) /* Valve V1, MLT output */

#define ALL_VALVES (V1b | V2b | V3b | V4b | V5b | V6b | V7b | V8b)
#define ALL_PUMPS  (P0b | P1b)

//------------------------------------------------------------------------------
// Heating options for HLT and Boil-kettle
// Note: These defines should correspond to the Arduino firmware
// Note: These defines are related to the tankOptions defines in the Tank object
//------------------------------------------------------------------------------
#define GAS_MODULATING     (0x01) /* Modulating gas-burner */
#define GAS_NON_MODULATING (0x02) /* Non-modulating (on/off) gas-burner */
#define ELECTRIC_HEATING1  (0x04) /* Electrical heating, SSR/Triac controlled */
#define ELECTRIC_HEATING2  (0x08) /* Electrical heating, SSR/Triac controlled */
#define ELECTRIC_HEATING3  (0x10) /* Electrical heating, SSR/Triac controlled */
#define HEATING_SOURCES    (0x1F) /* All energy-sources */

//-----------------------------------------------------
// Sensor options for HLT and MLT temperature sensors
//-----------------------------------------------------
#define TSENSOR_AVERAGING (0) /* Use both I2C and OW sensors for kettle temp. */
#define TSENSOR_USE_I2C   (1) /* Only use I2C for kettle temp., OW sensor is used for other temp. */
#define TSENSOR_USE_OW    (2) /* Only use OW for kettle temp., I2C sensor is not used */

//-----------------------------------------------------
// Struct for temperature-time pairs during mashing
//-----------------------------------------------------
typedef struct _mash_schedule
{
   qreal   time;       /* time (min.) to remain at this temperature */
   qreal   temp;       /* temperature (Celsius) to hold */
   int     timer;      /* timer, init. to NOT_STARTED */
   int     preht;      /* preheat timer */
   QString time_stamp; /* time when timer was started */
} mash_schedule;

//-----------------------------------------------------
// Struct for hop-gift pairs during boiling
//-----------------------------------------------------
typedef struct _hop_gift
{
    QList<int> time;   // time (min.) for a hop-gift
    QString    hops;   // description for the hop-gift
    QCheckBox  cb;     // checkbox
} hop_gift;

//------------------------------------------------------------------------------------------
// CLASS MainEbrew
//------------------------------------------------------------------------------------------
class MainEbrew : public QMainWindow
{
    Q_OBJECT

public:
    MainEbrew(void);             // Default constructor for MainEbrew

    Tank        *hlt;            // HLT-Tank object
    Tank        *mlt;            // MLT-Tank object
    Tank        *boil;           // Boil-kettle tank object
    Valve       *V1;             // Valve V1
    Valve       *V2;             // Valve V2
    Valve       *V3;             // Valve V3
    Valve       *V4;             // Valve V5
    Valve       *V6;             // Valve V6
    Valve       *V7;             // Valve V7
    Pump        *P1;             // Pump P1, main brew-pump
    Pump        *P2;             // Pump P2, pump for HLT heat-exchanger
    Meter       *F1;             // Flowmeter 1: between HLT-output and pump-input
    Meter       *F2;             // Flowmeter 2: Boil-kettle input
    Meter       *F3;             // Flowmeter 3: CFC-output
    Meter       *F4;             // Flowmeter 4: at MLT top return-manifold
    Meter       *T3;             // Temp. meter 3: Tcfc, other temp. meters are inside Tank objects
    Meter       *T4;             // Temp. meter 4: Thlt-ow, 2nd hlt temp or aux. temp. 1
    Meter       *T5;             // Temp. meter 5: Tmlt-ow, 2nd mlt temp or aux. temp. 2
    Display     *stdText;        // STD state description with sub-text
    Display     *autoManualText; // Label with Auto-All or Manual warning
    PowerButton *hltPid;         // HLT PID on/off powerButton
    PowerButton *boilPid;        // Boil-kettle PID on/off powerButton

    SlopeLimiter *slopeLimHLT;   // Slope-limiter object for tset_hlt
    SlopeLimiter *slopeLimBK;    // Slope-limiter object for tset_boil
    PidCtrl      *PidCtrlHlt;    // PID-controller object for HLT
    PidCtrl      *PidCtrlBk;     // PID-controller object for Boil-kettle

    QSettings   *RegEbrew;       // Registry Ebrew object
    Scheduler   *schedulerEbrew; // Scheduler object
    QSerialPort *serialPort;     // SerialPort object
    QUdpSocket  *udpSocket;      // Udp-socket object
    QFile       *fEbrewLog;      // Log-file object
    QFile       *fDbgCom;        // Com-port debug file object
    QToolBar    *toolBarB;       // Toolbar with brewing checkboxes
    QToolBar    *toolBarC;       // Toolbar with CIP checkboxes
    QString     ebrewRevision = EBREW_REVISION; // Ebrew SW revision number

    // Pointers to pipes and elbows in graphical scene
    Pipe *elbowP20; // Pump P2 top-left elbow
    Pipe *elbowP21; // Pump P2 bottom-left elbow
    Pipe *elbowP22; // Pump P2 bottom-right elbow
    Pipe *elbowP23; // Pump P2 top-right elbow
    Pipe *pipeH1;   // Pump P2 horizontal pipe
    Pipe *pipeH2;   // Input : horizontal pipe between TPipe1  and TPipe2
    Pipe *pipeH3;   // Input : horizontal pipe between TPipe2  and elbow3/valve3/BK-output
    Pipe *pipeH4;   // Input : horizontal pipe between elbow7  and pump P1
    Pipe *pipeH5;   // Output: horizontal pipe between elbow10 and TPipe3
    Pipe *pipeH6;   // Output: horizontal pipe between TPipe3  and Tpipe4
    Pipe *pipeH7;   // Output: horizontal pipe at CFC-output
    Pipe *pipeH8;   // Output: horizontal pipe between flow2   and elbow4
    Pipe *pipeH9;   // Output: horizontal pipe between HLT heat-exchanger input-pipe and elbow11
    Pipe *pipeH10;  // Output: horizontal pipe between Meter5 and MLT top-left return-manifold

    Pipe *pipeV1;   // Input : vertical pipe between TPipe1 and elbow7
    Pipe *pipeV2;   // Output: vertical pipe between valve4 and elbow11
    Pipe *pipeV3;   // Output: vertical pipe between valve4 and elbow10
    Pipe *pipeV4;   // Output: vertical pipe between valve6 and flow3
    Pipe *pipeV5;   // Output: vertical pipe between elbow4 and valve7

    Pipe *elbow2;   // Input : elbow between valve2  and flow1
    Pipe *elbow3;   // Input : elbow between valve3  and pipeH3
    Pipe *elbow4;   // Output: elbow between pipeH8  and elbow4
    Pipe *elbow5;   // Output: elbow between flow3   and temp3
    Pipe *elbow6;   // Output: elbow between pump P1 and Tpipe3
    Pipe *elbow7;   // Input : elbow between pipeV1  and pipeH4
    Pipe *elbow8;   // Output: elbow between HLT heat-exchanger output and flow4
    Pipe *elbow9;   // Output: elbow between flow4   and MLT top return manifold
    Pipe *elbow10;  // Output: elbow between pipeH5  and pipeV3
    Pipe *elbow11;  // Output: elbow between pipeV2  and pipeH9

    Pipe *Tpipe1;   // Input : connects to flow1 , pipeH2 and pipeV1
    Pipe *Tpipe2;   // Input : connects to pipeH2, valve1 and pipeH3
    Pipe *Tpipe3;   // Output: connects to pipeH5, elbow6 and pipeH6
    Pipe *Tpipe4;   // Output: connects to pipeH6, valve6 and flow2

    uint16_t   stateMachine(void);          // Ebrew State Transition Diagram
    void       readMashSchemeFile(bool initTimers); // Read mash-scheme from file
    void       setKettleVolumes(void);      // Set title of kettles with volumes and min. volume from Registry
    void       splitIpAddressPort(void);    // Split Registry variable into IP-address and port-number
    void       createRegistry(void);        // Create default Registry entries for Ebrew
    void       createStatusBar(void);       // Creates a status bar at the bottom of the screen
    void       updateMsIdxStatusBar(void);  // Update ms_idx in Statusbar
    void       updateSpIdxStatusBar(void);  // Update sp_idx in Statusbar
    void       createMenuBar(void);         // Creates a menu bar at the top of the screen
    void       initBrewDaySettings(void);   // Update brew-day settings from Registry values
    void       sleep(uint16_t msec);        // Sleep msec milliseconds
    void       commPortOpen(void);          // Open communications channel
    void       commPortClose(void);         // Close the communications channel
    void       commPortWrite(QByteArray s); // Writes a string to the communications channel
    void       setTopToolBar(int option);   // Set toolbar at top of screen for brewing or for CIP

    // Temperature, Volume and pid-output values
    qreal thlt  = 20.0;     // HLT actual temperature
    qreal tmlt  = 20.0;     // MLT actual temperature
    qreal tboil = 20.0;     // Boil-kettle actual temperature
    qreal tcfc;             // CFC-output actual temperature
    qreal thlt_i2c;         // HLT actual temperature, I2C-sensor
    qreal thlt_ow;          // HLT actual temperature, OW-sensor
    qreal tmlt_i2c;         // MLT actual temperature, I2C-sensor
    qreal tmlt_ow;          // MLT actual temperature, OW-sensor
    qreal ttriac;           // Temperature of Power Electronics
    qreal gamma_hlt;        // PID controller output for HLT
    qreal gamma_boil;       // PID controller output for Boil-kettle
    qreal tset_hlt;         // HLT reference temperature
    qreal tset_mlt;         // MLT reference temperature
    qreal tset_boil;        // HLT reference temperature
    qreal Vhlt = 0.0;       // Volume of HLT in litres
    qreal Vhlt_init;        // Initial water volume in HLT
    qreal Vmlt = 0.0;       // Volume of MLT in litres
    qreal Vboil = 0.0;      // Volume of Boil kettle in litres
    qreal Vhlt_old;         // Prev. value of Vhlt, used in STD
    qreal Vmlt_old;         // Prev. value of Vmlt, used in STD
    qreal Vboil_old;        // Prev. value of Vboil, used in STD

    // Flow-rate values
    qreal FlowHltMlt;           // Flow-meter 1 value
    qreal FlowMltBoil;          // Flow-meter 2 value
    qreal FlowCfcOut;           // Flow-meter 3 value
    qreal Flow4;                // Flow-meter 4 value
    qreal FlowCfcOutResetValue = 0.0; // Needed in boiling phase to reset flowmeter
    bool  flow1Running;         // True = flowsensor 1 should see a flow
    bool  flow2Running;         // True = flowsensor 2 should see a flow
    bool  flow3Running;         // True = flowsensor 3 should see a flow
    bool  flow4Running;         // True = flowsensor 4 should see a flow
    int   sensorAlarmInfo;      // alarm bits for temp. and flow-sensors
    int   alarmSound = ALARM_TEMP_FLOW_SENSORS; // alarm on all sensor errors

    bool  triacTooHot;           // true = SSR too hot
    bool  hltGasNonMod = false;  // true = enable non-modulating gas-valve
    bool  hltGasMod = false;     // true = enable modulating gas-valve
    bool  boilGasNonMod = false; // true = enable non-modulating gas-valve
    bool  boilGasMod = false;    // true = enable modulating gas-valve

    /* Switches and Fixes for variables */
    bool  tset_hlt_sw  = false;  // Switch value for tset_hlt
    qreal tset_hlt_fx  = 0.0;    // Fix value for tset_hlt
    bool  tset_boil_sw = false;  // Switch value for tset_boil
    qreal tset_boil_fx = 0.0;    // Fix value for tset_boil
    bool  gamma_hlt_sw = false;  // Switch value for gamma-hlt
    qreal gamma_hlt_fx = 0.0;    // Fix value for gamma-hlt
    bool  gamma_boil_sw = false; // Switch value for gamma-boil
    qreal gamma_boil_fx = 0.0;   // Fix value for gamma-boil

    bool  thlt_sw   = false;     // Switch value for Thlt
    qreal thlt_fx   = 0.0;       // Fix value for Thlt [Celsius]
    bool  tmlt_sw   = false;     // Switch value for Tmlt
    qreal tmlt_fx   = 0.0;       // Fix value for Tmlt [Celsius]
    bool  tboil_sw  = false;     // Switch value for Tboil
    qreal tboil_fx  = 0.0;       // Fix value for Tboil [Celsius]
    bool  ttriac_sw = false;     // Switch value for Ttriac
    qreal ttriac_fx = 0.0;       // Fix value for Ttriac

    bool  vhlt_sw  = false;      // Switch value for Vhlt
    qreal vhlt_fx  = 0.0;        // Fix value for Vhlt
    bool  vmlt_sw  = false;      // Switch value for Vmlt
    qreal vmlt_fx  = 0.0;        // Fix value for Vmlt
    bool  vboil_sw = false;      // Switch value for Vboil
    qreal vboil_fx = 0.0;        // Fix value for Vboil

    bool  delayedStart = false;  // true = HLT PID controller is started at a fixed time and date
    QDateTime dlyStartTime;      // Holds date and time for delayed-start

    // State Transition Diagram (STD) values
    int   ebrew_std  = S00_INITIALISATION; // Current state of STD
    int   ms_tot     = 0;         // total nr. of valid temp & time values
    int   ms_idx     = 0;         // index in ms[] array
    int   sp_idx     = 0;         // Sparging index [0..sps->sp_batches-1]
    int   timer1     = 0;         // Timer for state 'Sparging Rest'
    int   timer2     = 0;         // Timer for state 'Delay_xSEC'
    int   timer3     = 0;         // Timer for state 'Pump Pre-Fill'
    int   mrest_tmr  = 0;         // Timer for state 'Mast Rest 5 Min.'
    int   brest_tmr  = 0;         // Timer for state 'Boiling finished, prepare Chiller'
    int   timer5     = 0;         // Timer for state 'Boiling'
    int   cip_tmr1   = 0;         // Timer for CIP process
    int   cip_circ   = 0;         // Counter for CIP circulations

    /* Mash Settings */
    int   ph_time;                // ph_time in seconds, PREHEAT_TIME in minutes

    /* Sparge Settings */
    int   mash_vol;               // Total mashing volume in litres (read from maisch.sch)
    int   sp_vol;                 // Total sparge volume in litres (read from maisch.sch)
    int   sp_time_ticks;          // sp_time in TS ticks
    int   boil_time_ticks;        // boil_time in TS ticks
    qreal sp_vol_batch;           // Sparge volume of 1 batch = sp_vol / sp_batches
    qreal sp_vol_batch0;          // Sparge volume of first batch

    /* Boil Settings */
    int   boil_time;              // Total boiling time in minutes (read from maisch.sch)

    mash_schedule      ms[MAX_MS];      // struct containing mash-schedule
    hop_gift           hops[MAX_HOPS];  // struct containing hop-gift times and descriptions
    QList<int>         hopTimes;        // List with hop-times in minutes
    QStringList        hopTexts;        // List with hop descriptions
    bool               hopCb[MAX_HOPS]; // List with checkboxes
    int                hopIdx = 0;      // Index in hop-times
    QCheckBox          cbHops;          // Checkbox for msgBox()

    /* Communications channel variables */
    bool         ReadDataAvailable; // true = ReadData is available, set by CommPortRead() slot
    QByteArray   ReadData;          // Data read from virtual COM port by CommPortRead() slot
    bool         comPortIsOpen;     // true = communication channel is opened
    QHostAddress ipAddress;         // IP-address of Ebrew hardware in Reqistry
    int          ipPort;            // Port number of Ebrew hardware in Registry
    QHostAddress ebrewHwIp;         // Actual IP-address of Ebrew hardware

    /* Time-stamps for Sparge, Boil and Chilling*/
    QStringList burner_on; // Burner-on time-stamp
    QStringList mlt2boil;  // Strings for time-stamp moment of MLT -> BOIL
    QStringList hlt2mlt;   // Strings for time-stamp moment of HLT -> MLT
    QStringList Boil;      // Boil-start and Boil-End time-stamps
    QStringList Chill;     // Chill-start and Chill-End time-stamps

public slots:
    void     task_alive_led(void);             // 500 msec. task for blinking alive LED
    void     task_read_temps(void);            // 2 sec. task for reading all temperatures
    void     task_pid_control(void);           // TS sec. task for PID-controllers
    void     task_update_std(void);            // 1 sec. task for call to STD
    void     task_read_flows(void);            // 2 sec. task for reading all flow-sensors
    void     task_write_logfile(void);         // 5 sec. task for writing info into logfile
    void     about(void);                      // About() screen of MainEbrew
    void     MenuEditMashScheme(void);         // Edit->Mash Scheme dialog screen
    void     MenuEditFixParameters(void);      // Edit->Fix Parameters dialog screen
    void     MenuEditTerminal(void);           // Edit->Terminal editor dialog screen
    void     MenuViewProgress(void);           // View->Mash & Sparge Progress dialog screen
    void     MenuViewStatusAlarms(void);       // View->Status & Alarms dialog screen
    void     MenuViewTaskList(void);           // View->Task-list and Timings dialog screen
    void     MenuOptionsPidSettings(void);     // Options->PID Settings dialog screen
    void     MenuOptionsMeasurements(void);    // Options->Measurements Settings dialog screen
    void     MenuOptionsBrewDaySettings(void); // Options->Brew Day Settings dialog screen
    void     MenuOptionsSystemSettings(void);  // Options->System Settings dialog screen
    void     commPortRead(void);               // Reads a string from the communications channel
    void     removeLF(QByteArray& s);          // Removes \n from QByteArray

protected:
    void     closeEvent(QCloseEvent *event);   // called when application is closed
    void     keyPressEvent(QKeyEvent *event);  // called when a key is pressed
    void     msgBox(QString title, QString text, QCheckBox *cb); /* shows only one instance of a MessageBox */

    int      pidCntr       = 0;                // counter for task_pid_control()
    bool     toggleLed     = false;            // Indicator for Alive LED
    QString  line1MashScheme;                  // Title line in mash-scheme file, needed for log-file

private:
    // Labels in Statusbar at bottom of screen
    QLabel    *statusAlarm;       // Statusbar Sensor Alarm label
    QLabel    *statusMashScheme;  // Statusbar Mash-scheme label
    QLabel    *statusHops;        // Statusbar Hop-gift times
    QLabel    *statusMashVol;     // Statusbar Mash-volume value
    QLabel    *statusSpargeVol;   // Statusbar Sparge-volume value
    QLabel    *statusBoilTime;    // Statusbar total boil-time value
    QLabel    *statusMsIdx;       // Statusbar mash-index (ms_idx) value
    QLabel    *statusSpIdx;       // Statusbar sparge-index (sp_idx) value
    QLabel    *statusSwRev;       // Statusbar SW+HW revision numbers

    // Checkboxes at toolbar at top of screen
    QCheckBox *toolStartCIP;      // Toolbar top checkbox start CIP program
    QCheckBox *toolStartAddMalt;  // Toolbar top checkbox ready to add malt
    QCheckBox *toolMaltAdded;     // Toolbar top checkbox malt is added
    QCheckBox *toolMLTEmpty;      // Toolbar top checkbox MLT is empty
    QCheckBox *toolBoilStarted;   // Toolbar top checkbox boiling is started
    QCheckBox *toolStartChilling; // Toolbar top checkbox prepare chiller
    QCheckBox *toolReadyChilling; // Toolbar top checkbox chilling is finished
    QCheckBox *toolCipInitDone;   // Toolbar top checkbox ready to start CIP program
    QCheckBox *toolCipDrainBK;    // Toolbar top checkbox drain Boil-kettle
    QCheckBox *toolCipHltFilled;  // Toolbar top checkbox HLT filled with water
    QCheckBox *toolGFSpargeWater; // Toolbar top checkbox Grainfather Sparge Water heater
}; // MainEbrew()

#endif // MAIN_EBREW_H
