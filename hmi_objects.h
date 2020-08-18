/**************************************************************************************
** Filename    : hmi_objects.h
** Author      : Emile
** Purpose     : Header file for hmi_objects.cpp. Contains all graphical objects that
**               are used by MainEbrew:
**               PowerButton: A push-button with a green/red LED in it.
**               Meter      : a flowmeter or temperaturesensor, showing actual values.
**               Tank       : a tank object for constructing a HLT, MLT or boil-kettle.
**               Pipe       : a pipe which is used to connect everything. A pipe can have
**                            just 2 pipes, but also 3 and 4 pipes.
**               Display    : A display with a sub-text for displaying the actual state.
**               Actuator   : A base object for actuators
**               Valve      : A valve, derived from Actuator, for blocking or enabling a flow.
**               Pump       : A pump, derived from Actuator, for pumping fluids through pipes.
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
#ifndef HMI_OBJECTS_H
#define HMI_OBJECTS_H

#include <QGraphicsItem>
#include <QPainter>
#include <QString>
#include <QAction>
#include <QMenu>
#include <QGraphicsSimpleTextItem>
#include <QPushButton>
#include "controlobjects.h"

#define HORIZONTAL      (0) /* Valve horizontal direction */
#define VERTICAL        (1) /* Valve vertical direction */
#define OUT_RIGHT       (0) /* Pump output on right */
#define OUT_LEFT        (1) /* Pump output on left */

#define METER_HFLOW     (0) /* Meter-type: flow-meter horizontal lay-out */
#define METER_HTEMP     (1) /* Meter-type: temperature-meter horizontal lay-out */
#define METER_VFLOW     (2) /* Meter-type: flow-meter vertical lay-out */
#define METER_VTEMP     (3) /* Meter-type: temperature-meter vertical lay-out */
#define METER_FLOW_MA_N (5) /* Flow-meter moving-average filter order */
#define FLOWRATE_RAW    (0) /* get unfiltered flow-rate */
#define FLOWRATE_FIL    (1) /* get filtered flow-rate */

#define RPIPE          (10) /* Radius of pipes */
#define RPUMP          (35) /* Radius of Pump */
#define VALVE_SIZE     (60) /* Valve size */
#define RVALVE         (12) /* Radius of ball in middle of valve */
#define TANK_WALL      (10) /* Thickness of tank wall */

#define PIPE2_LEFT_TOP     (0)
#define PIPE2_LEFT_RIGHT   (1)
#define PIPE2_LEFT_BOTTOM  (2)
#define PIPE2_TOP_RIGHT    (3)
#define PIPE2_TOP_BOTTOM   (4)
#define PIPE2_BOTTOM_RIGHT (5)
#define PIPE3_NO_TOP       (6)
#define PIPE3_NO_RIGHT     (7)
#define PIPE3_NO_BOTTOM    (8)
#define PIPE3_NO_LEFT      (9)
#define PIPE4_ALL         (10)
#define PIPE2_RIGHT_LEFT  (11)
#define PIPE2_BOTTOM_TOP  (12)
#define PIPE2_BOTTOM_TOP_NO_ARROW (13)

// These are the possible tank options
#define TANK_HEAT_EXCHANGER   (1) /* Tank contains heat-exchanger with pipes on the right */
#define TANK_MANIFOLD_BOTTOM  (2) /* False bottom connecting to TANK_EXIT_BOTTOM */
#define TANK_MANIFOLD_TOP     (4) /* Return manifold at top-left of tank */
#define TANK_RETURN_BOTTOM    (8) /* Return pipe at bottom of tank */
#define TANK_EXIT_BOTTOM     (16) /* One pipe output at tank bottom */

// Possible states for Actuators (pumps, valves)
#define MANUAL_OFF (0) /* Actuator in manual mode, off */
#define MANUAL_ON  (1) /* Actuator in manual mode, on */
#define AUTO_OFF   (2) /* Actuator in automatic mode, off */
#define AUTO_ON    (3) /* Actuator in automatic mode, on */

// Coordinates for Tank object
#define COORD_LEFT_PIPE1    (0)
#define COORD_LEFT_PIPE2    (1)
#define COORD_LEFT_TOP_PIPE (2)
#define COORD_BOTTOM_PIPE1  (3)
#define COORD_BOTTOM_PIPE2  (4)
#define COORD_RIGHT_PIPE1   (5)
#define COORD_RIGHT_PIPE2   (6)

// Coordinates for Valve, Pipe and Pump objects
#define COORD_LEFT          (7)
#define COORD_RIGHT         (8)
#define COORD_TOP           (9)
#define COORD_BOTTOM       (10)

// Color-definitions for input and output pipes, with and without flow
#define COLOR_OUT0 QColor(  0,  0,255) /* Output pipe color when no flow */
#define COLOR_OUT1 QColor(  0,220,255) /* Output pipe color when flow */
#define COLOR_IN0  QColor(102,153,255) /* Input pipe color when no flow */
#define COLOR_IN1  QColor(102,255,255) /* Input pipe color when flow */
#define COLOR_LEFT_PIPES   (0)
#define COLOR_BOTTOM_PIPE1 (1)
#define COLOR_BOTTOM_PIPE2 (2)
#define COLOR_RIGHT_PIPES  (3)
#define COLOR_TOP_PIPE     (4)

// Value returned from Ebrew hardware when a temperaturesensor is faulty or not connected
#define SENSOR_VAL_LIM_OK (-99.9)
#define TEMP_DEFAULT       (20.0)

//------------------------------------------------------------------------------------------
// Pushbutton with a green/red LED
//------------------------------------------------------------------------------------------
class PowerButton : public QPushButton
{
    Q_OBJECT

public:
    PowerButton(int x, int y, int width, int height, QString name);
    bool getButtonState(void);
    void setButtonState(bool state);

public slots:
    void button_pressed(void);

protected:
    bool    buttonState;
    QPixmap pixmap_off;
    QPixmap pixmap_on;
    QIcon   icon_off;
    QIcon   icon_on;
}; // class Powerbutton

//------------------------------------------------------------------------------------------
// Meter object for Flow-meter and Temp-meter
//------------------------------------------------------------------------------------------
class Meter : public QGraphicsSimpleTextItem
{
public:
    Meter(QPointF point, uint8_t type, QString name);
    void    setName(QString name);
    void    setTempValue(qreal value);
    void    setFlowValue(qreal value,qreal temp); /* temp. is needed for temp. correction */
    void    setFlowParameters(uint16_t msec, bool temp_corr, qreal flow_err);
    qreal   getFlowRate(uint8_t fil); // return the (un)filtered flow-rate in L/min.
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void    setError(bool err);
    QPointF getCoordinates(uint8_t side);
    void    initFlowRateDetector(uint8_t perc);
    bool    isFlowRateLow(void);
    QRectF  boundary; // boundary of object

    // Variables for flowrate-low detector
    uint8_t frl_std;         // STD state number
    uint8_t frl_tmr;         // Timer value
    double  frl_det_lim;     // Lower-limit for flowrate
    double  frl_min_det_lim; // Minimum flowrate: sensor-check
    uint8_t frl_perc;        // Percentage of max flowrate

protected:
    QRectF boundingRect() const override { return boundary; }
    QPointF left,top,right,bottom; // coordinates of various points
    QString meterName;       // Name of meter, e.g. Flow1
    qreal   meterValue;      // Actual meter value
    qreal   meterValueOld;   // Previous meter value
    uint8_t meterType;       // Flow or Temp. meter
    bool    meterError;      // true = error (painted red)
    // Variables for flowrate calculation
    qreal   Ts;              // Time in msec. between two setValue() calls
    bool    tempCorrection;  // true = apply temperature volume correction
    qreal   flowErr;         // error correction percentage for flowmeter
    qreal   flowRate;        // Filtered flow-rate in L/min.
    qreal   flowRateRaw;     // Un-filtered flow-rate in L/min.
    MA      *pma;            // pointer to moving-average filter for flowrate
}; // Class Meter

//------------------------------------------------------------------------------------------
// Tank object for HLT, MLT and Boil-kettle
//------------------------------------------------------------------------------------------
class Tank : public QGraphicsPolygonItem
{
public:
    Tank(int x, int y, int width, int height, uint8_t options, QString name);
    void    setOrientation(int width, int height, uint8_t options);
    void    setName(QString name);
    void    setValues(qreal temp, qreal sp, qreal vol, qreal power);
    void    setColor(uint8_t pipe, QColor color);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF  boundingRect() const override { return boundary; }
    QPointF getCoordinates(int which);

protected:
    int       tankWidth, tankHeight;
    QString   tankName;       /* Tank name */
    uint8_t   tankOptions;    /* Tank options: TANK_HEAT_EXCHANGER ... TANK_EXIT_BOTTOM */
    QPolygonF tankPolygon;    /* Contains polygon for drawing the tank */
    qreal     tankTemp;       /* Actual temperature inside tank */
    qreal     tankSetPoint;   /* Setpoint temperature for tank */
    qreal     tankVolume;     /* Actual volume in the tank */
    qreal     tankPower;      /* Actual heating power applied to the tank */
    bool      tankTempErr;    /* true = error in actual temperature */
    QRectF    boundary;       /* boundary of tank object */

    QPointF   leftPipe1;      /* Coordinate of top-left pipe for connecting a pump */
    QPointF   leftPipe2;      /* Coordinate of bottom-left pipe for connecting a pump */
    QPointF   leftTopPipe;    /* Coordinate of top-left pipe for return manifold at top of tank */
    QPointF   bottomPipe1;    /* Coordinate of bottom pipe for manifold at bottom of tank */
    QPointF   bottomPipe2;    /* Coordinate of bottom return pipe at bottom of tank */
    QPointF   rightPipe1;     /* Coordinate of top-right pipe for heat-exchanger in tank */
    QPointF   rightPipe2;     /* Coordinate of bottom-right pipe for heat-exchanger in tank */
    QColor    colLeftPipes;   /* Color of pipes at left of (HLT) tank */
    QColor    colBottomPipe1; /* Color of output-pipe at bottom of tank */
    QColor    colBottomPipe2; /* Color of input-pipe at bottom of tank */
    QColor    colRightPipes;  /* Color of heat-exchanger pipes at right-side of tank */
    QColor    colTopPipe;     /* Color of input-pipe at top of tank */
}; // class Tank

//------------------------------------------------------------------------------------------
// Pipe object
//------------------------------------------------------------------------------------------
class Pipe : public QGraphicsPolygonItem
{
public:
    Pipe(int x, int y, uint8_t type, uint16_t length, QColor color);
    Pipe(QPointF point, uint8_t type, uint16_t length, QColor color);
    void    drawPipe(uint8_t type, uint16_t length, QColor color);
    QPointF getCoordinates(uint8_t side);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void    setColor(QColor color);

protected:
    QRectF    boundingRect() const override { return boundary; }
    uint16_t  pipeLength;
    uint8_t   pipeType;
    QPolygonF pipePolygon; /* Contains polygon for drawing the pipe */
    QPointF   left,top,right,bottom; // coordinates of various points
    QColor    pipeColor;
    QRectF    boundary; // boundary of object
}; // class Pipe

//------------------------------------------------------------------------------------------
// Text-display object for STD with sub-text
//------------------------------------------------------------------------------------------
class Display : public QGraphicsSimpleTextItem
{
public:
    Display(QPointF point, int w);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF  boundary; // boundary of object
    void    setSubText(QString string);

protected:
    QRectF  boundingRect() const override { return boundary; }
    QString subText;
    int     width;
    int     height;
}; // Class Display

//------------------------------------------------------------------------------------------
// Base object for Valves and Pumps
//------------------------------------------------------------------------------------------
class Actuator : public QGraphicsPolygonItem
{
public:
    Actuator(QPointF point, QString name);
    void    setColor(QColor color);
    void    setName(QString name);
    void    setStatus(uint8_t status);
    void    setNextStatus(void);
    uint8_t getStatus(void);
    QPointF getCoordinates(uint8_t side);
    bool    inManualMode(void);
    void    setActuator(uint16_t& actionBits, uint16_t whichBit);
    QRectF  boundary; // boundary of object

protected:
    void      contextMenuEvent(QGraphicsSceneContextMenuEvent * event) override;
    QString   actuatorName;                     // actuator name
    QColor    actuatorColor       = Qt::red;    // color of actuator
    bool      actuatorOrientation = HORIZONTAL; // Orientation of actuator: HORIZONTAL or VERTICAL
    QPolygonF actuatorPolygon;                  // Contains polygon for drawing the actuator
    uint8_t   actuatorStatus;                   // Actuator status: MANUAL_OFF, MANUAL_ON, AUTO_OFF, AUTO_ON
    QString   statustext[4] = { "OFF(M)", "ON (M)", "OFF(A)", "ON (A)" };
    QPointF   left,top,right,bottom;            // coordinates of various points
}; // class Actuator

//------------------------------------------------------------------------------------------
// Valve object, derived from Actuator
//------------------------------------------------------------------------------------------
class Valve : public Actuator
{
public:
    Valve(QPointF point, bool orientation, QString name);
    void setOrientation(bool orientation); // draws the object as a polygon

protected:
    QRectF boundingRect() const override { return boundary; }
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override; // overriding paint()

private:
}; // class Valve

//------------------------------------------------------------------------------------------
// Pump object, derived from Actuator
//------------------------------------------------------------------------------------------
class Pump : public Actuator
{
public:
    Pump(QPointF point, bool orientation, QString name);
    void setPumpOrientation(bool orientation); // draws the object as a polygon

protected:
    QRectF boundingRect() const override { return boundary; }
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override; // overriding paint()

private:
}; // class Pump

#endif // HMI_OBJECTS_H
