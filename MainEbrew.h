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

#include "hmi_objects.h"
#include "controlobjects.h"
#include "scheduler.h"

#define COMMDBGFILE "com_port_dbg.txt"
#define LOGFILE     "ebrewlog.txt"
#define MASHFILE    "maisch.sch"
#define REGKEY      "HKEY_CURRENT_USER\\Software\\ebrew\\V3"
#define EBREW_HW_ID "E-Brew"

//------------------------------
// Defines for Audio Alarm
//------------------------------
#define ALARM_OFF               (0) /* No sound */
#define ALARM_TEMP_SENSORS      (1) /* Audio alarm only on Temp. sensor error */
#define ALARM_FLOW_SENSORS      (2) /* Audio alarm only on flowsensor error */
#define ALARM_TEMP_FLOW_SENSORS (3) /* Audio alarm on all sensor errors */

//-------------------------------
// Defines for sensor_alarm_info
//-------------------------------
#define SENS_THLT   (0x01)
#define SENS_TMLT   (0x02)
#define SENS_TBOIL  (0x04)
#define SENS_TCFC   (0x08)
#define SENS_FLOW1  (0x10) /* HLT -> MLT */
#define SENS_FLOW2  (0x20) /* MLT -> Boil */
#define SENS_FLOW3  (0x40) /* CFC out */
#define SENS_FLOW4  (0x80) /* Reserved */

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
#define TS_STD_MSEC   (1000) /* Call state-machine every second */
#define TS_LED_MSEC    (500) /* Alive LED blinking every 1/2 sec. */
#define TS_WR_LOGFILE (5000) /* Write to logfile every 5 seconds */

//----------------------------------
// Defines for Brew-day Settings
//----------------------------------
#define MAX_MS      (10) /* Max. number of mash temp-time pairs */
#define MAX_SP      (10) /* Max. number of batch sparge sessions */
#define NOT_STARTED (-1) /* Timer not started */

//----------------------------------
// Defines for readMashSchemeFile()
//----------------------------------
#define INIT_TIMERS    (true)
#define NO_INIT_TIMERS (false)

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

#define S09_EMPTY_MLT              (9)
#define S10_WAIT_FOR_BOIL         (10)
#define S11_BOILING               (11)
#define S12_BOILING_FINISHED      (12)
#define S16_CHILL_PUMP_FERMENTOR  (16)
#define S17_FINISHED              (17)
#define S18_MASH_REST_5_MIN       (18)
#define S19_RDY_TO_ADD_MALT       (19)

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
#define STD_MAX                   (S32_CIP_END)

//-------------------------------------------------------------
// Hard-coded Timers.
// task update_std() runs every second, so 1 second is 1 tick.
//-------------------------------------------------------------
#define TMR_PREFILL_PUMP       (60)
#define TMR_DELAY_xSEC         (10)
#define TMR_MASH_REST_5_MIN   (300)
#define TMR_BOIL_REST_5_MIN   (300)

//--------------------------------------------------------------------------
// #defines for the valves. Each valve can be set manually or automatically
// by the STD. Bit-values are for the variable 'valves'.
//--------------------------------------------------------------------------
#define P1b  (0x0200)
#define P0b  (0x0100)
#define V8b  (0x0080)
#define V7b  (0x0040)
#define V6b  (0x0020)
#define V5b  (0x0010)
#define V4b  (0x0008)
#define V3b  (0x0004)
#define V2b  (0x0002)
#define V1b  (0x0001)

#define ALL_VALVES (V1b | V2b | V3b | V4b | V5b | V6b | V7b | V8b)
#define ALL_PUMPS  (P0b | P1b)
#define ALL_MANUAL (V1M | V2M | V3M | V4M | V5M | V6M | V7M | V8M | P0M | P1M)

//-----------------------------
// E-brew System Modes
//-----------------------------
#define GAS_MODULATING     (0)
#define GAS_NON_MODULATING (1)
#define ELECTRICAL_HEATING (2)

//-------------------------------
// Defines for sensor_alarm_info
//-------------------------------
#define SENS_THLT   (0x01)
#define SENS_TMLT   (0x02)
#define SENS_TBOIL  (0x04)
#define SENS_TCFC   (0x08)
#define SENS_FLOW1  (0x10) /* HLT -> MLT */
#define SENS_FLOW2  (0x20) /* MLT -> Boil */
#define SENS_FLOW3  (0x40) /* CFC out */
#define SENS_FLOW4  (0x80) /* Reserved */

typedef struct _mash_schedule
{
   qreal   time;       /* time (min.) to remain at this temperature */
   qreal   temp;       /* temperature (Celsius) to hold */
   int     timer;      /* timer, init. to NOT_STARTED */
   int     preht;      /* preheat timer */
   QString time_stamp; /* time when timer was started */
} mash_schedule;

//------------------------------------------------------------------------------------------
class MainEbrew : public QMainWindow
{
    Q_OBJECT

public:
    MainEbrew(void);             // Default constructor for MainEbrew

    Tank        *hlt;            // Pointer to HLT object
    Tank        *mlt;            // Pointer to MLT object
    Tank        *boil;           // Pointer to Boil-kettle object
    Valve       *V1;             // Pointer to valve V1
    Valve       *V2;             // Pointer to valve V2
    Valve       *V3;             // Pointer to valve V3
    Valve       *V4;             // Pointer to valve V5
    Valve       *V6;             // Pointer to valve V6
    Valve       *V7;             // Pointer to valve V7
    Pump        *P1;             // Pointer to pump P1, main brew-pump
    Pump        *P2;             // Pointer to pump P2, pump for HLT heat-exchanger
    Meter       *F1;             // Flowmeter 1: between HLT-output and pump-input
    Meter       *F2;             // Flowmeter 2: Boil-kettle input
    Meter       *F3;             // Flowmeter 3: CFC-output
    Meter       *F4;             // Flowmeter 4: at MLT top return-manifold
    Meter       *T3;             // Temp. meter 3: Tcfc
    Display     *std_text;       // STD state description
    PowerButton *hlt_pid;        // HLT PID on/off powerButton
    PowerButton *boil_pid;       // Boil-kettle PID on/off powerButton
    QSettings   *RegEbrew;       // Pointer to Registry Ebrew object
    Scheduler   *schedulerEbrew; // Pointer to scheduler object
    QSerialPort *serialPort;     // Pointer to serialPort object
    QFile       *fEbrewLog;      // Pointer to log-file object
    QFile       *fDbgCom;        // Pointer to com-port debug file object

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

    uint16_t   state_machine(void);   // Ebrew State Transition Diagram
    void       readMashSchemeFile(bool initTimers);
    void       setKettleNames(void);  // Set title of kettles with volumes from Registry
    void       createRegistry(void);  // Create default Registry entries for Ebrew
    void       createStatusBar(void); // Creates a status bar at the bottom of the screen
    void       createMenuBar(void);   // Creates a menu bar at the top of the screen
    void       setStateName(void);    // Update state nr and description on screen
    void       initBrewDaySettings(void);   // Update brew-day settings from Registry values
    void       sleep(uint16_t msec);        // Sleep msec milliseconds
    void       commPortOpen(void);          // Open communications channel
    void       commPortClose(void);         // Close the communications channel
    void       commPortWrite(QByteArray s); // Writes a string to the communications channel

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
    int   ms_tot     = 0;     // total nr. of valid temp & time values
    int   ms_idx     = 0;     // index in ms[] array
    int   sp_idx     = 0;     // Sparging index [0..sps->sp_batches-1]
    int   timer1     = 0;     // Timer for state 'Sparging Rest'
    int   timer2     = 0;     // Timer for state 'Delay_xSEC'
    int   timer3     = 0;     // Timer for state 'Pump Pre-Fill'
    int   mrest_tmr  = 0;     // Timer for state 'Mast Rest 5 Min.'
    int   brest_tmr  = 0;     // Timer for state 'Boiling finished, prepare Chiller'
    int   timer5     = 0;     // Timer for state 'Boiling'
    int   mash_rest  = 0;     // 1 = mash rest after malt is added
    int   cip_tmr1   = 0;     // Timer for CIP process
    int   cip_circ   = 0;     // Counter for CIP circulations
    int   boil_rest  = 0;     // 1 = let wort rest after malt is added
    int   malt_first = 0;     // 1 = malt is added to MLT first, then water

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

    mash_schedule ms[MAX_MS];     // struct containing mash schedule
    SlopeLimiter  *slopeLimHLT;   // slope-limiter object for tset_hlt
    SlopeLimiter  *slopeLimBK;    // slope-limiter object for tset_boil
    PidCtrl       *PidCtrlHlt;    // PID-controller object for HLT
    PidCtrl       *PidCtrlBk;     // PID-controller object for Boil-kettle

    bool       ReadDataAvailable; // true = ReadData is available, set by CommPortRead() slot
    QByteArray ReadData;          // Data read from virtual COM port by CommPortRead() slot
    bool       comPortIsOpen;     // true = communication channel is opened

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
    void     MenuViewProgress(void);           // View->Mash & Sparge Progress dialog screen
    void     MenuViewTaskList(void);           // View->Task-list and Timings dialog screen
    void     MenuOptionsPidSettings(void);     // Options->PID Settings dialog screen
    void     MenuOptionsMeasurements(void);    // Options->Measurements Settings dialog screen
    void     MenuOptionsBrewDaySettings(void); // Options->Brew Day Settings dialog screen
    void     MenuOptionsSystemSettings(void);  // Options->System Settings dialog screen
    void     commPortRead(void);               // Reads a string from the communications channel

protected:
    void     closeEvent(QCloseEvent *event);   // called when application is closed
    void     keyPressEvent(QKeyEvent *event);  // called when a key is pressed
    void     removeLF(QByteArray& s);          // Removes \n from QByteArray
    void     msgBox(QString title, QString text, QCheckBox *cb);

    // Temperature, Volume and pid-output values
	qreal thlt;             // HLT actual temperature
	qreal thlt_offset;      // Offset to add to Thlt measurement
	qreal tmlt;             // MLT actual temperature
	qreal tmlt_offset;      // Offset to add to Tmlt measurement
	qreal tboil;            // Boil-kettle actual temperature
	qreal tboil_offset;     // Offset to add to Tboil measurement
	qreal tcfc;             // CFC-output actual temperature
	qreal tcfc_offset;      // Offset to add to Tcfc measurement
	qreal tset_slope_limit; // Slope limiter for Temp. Setpoints
    qreal ttriac;           // Temperature of Power Electronics
	qreal gamma_hlt;        // PID controller output for HLT
	qreal gamma_boil;       // PID controller output for Boil-Kettle
	qreal tset_hlt;         // HLT reference temperature
	qreal tset_mlt;         // MLT reference temperature
	qreal tset_boil;        // HLT reference temperature
    qreal Vhlt;             // Volume of HLT in litres
    qreal Vmlt;             // Volume of MLT in litres
    qreal Vboil;            // Volume of Boil kettle in litres
    qreal Vhlt_old;         // Prev. value of Vhlt, used in STD
    qreal Vmlt_old;         // Prev. value of Vmlt, used in STD
    qreal Vboil_old;        // Prev. value of Vboil, used in STD

    // Flow-rate values
    qreal Flow_hlt_mlt;         // Flow1
    qreal Flow_mlt_boil;        // Flow2
    qreal Flow_cfc_out;         // Flow3
    qreal Flow4;                // Flow4: Future Use
    qreal Flow_cfc_out_reset_value;
    bool  flow1Running;         // True = flowsensor 1 should see a flow
    bool  flow2Running;         // True = flowsensor 2 should see a flow
    bool  flow3Running;         // True = flowsensor 3 should see a flow
    bool  flow4Running;         // True = flowsensor 4 should see a flow
    int   sensorAlarmInfo;      // alarm bits for temp. and flow-sensors
    int   alarmSound = ALARM_TEMP_FLOW_SENSORS; // alarm on all sensor errors

    bool  toggle_led;           // Indicator for Alive LED
    bool  power_up_flag;        // true = power-up in progress
    bool  triac_too_hot;        // true = SSR too hot

    QString ebrew_revision = "$Revision: 3.00 $";
    QString line1MashScheme;    // Title line in mash-scheme file

private:
    // Pointers to Labels in Statusbar at bottom of screen
    QLabel    *statusAlarm;
    QLabel    *statusMashScheme;
    QLabel    *statusMashVol;
    QLabel    *statusSpargeVol;
    QLabel    *statusBoilTime;
    QLabel    *statusMsIdx;
    QLabel    *statusSpIdx;
    QLabel    *statusSwRev;
    QCheckBox *toolStartCIP;
    QCheckBox *toolStartAddMalt;
    QCheckBox *toolMaltAdded;
    QCheckBox *toolBoilStarted;
    QCheckBox *toolStartChilling;
    QCheckBox *toolCipInitDone;
    QCheckBox *toolCipDrainBK;
    QCheckBox *toolCipHltFilled;

}; // MainEbrew()

#endif // MAIN_EBREW_H
