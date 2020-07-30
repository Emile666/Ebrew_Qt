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

#define HORIZONTAL (0) /* Valve horizontal direction */
#define VERTICAL   (1) /* Valve vertical direction */
#define OUT_RIGHT  (0) /* Pump output on right */
#define OUT_LEFT   (1) /* TODO Pump output on left */

#define METER_HFLOW     (0) /* Meter-type: flow-meter horizontal lay-out */
#define METER_HTEMP     (1) /* Meter-type: temperature-meter horizontal lay-out */
#define METER_VFLOW     (2) /* Meter-type: flow-meter vertical lay-out */
#define METER_VTEMP     (3) /* Meter-type: temperature-meter vertical lay-out */
#define METER_FLOW_MA_N (5) /* Flow-meter moving-average filter order */

#define RPIPE      (10) /* Radius of pipes */
#define RPUMP      (35) /* Radius of Pump */
#define VALVE_SIZE (60) /* Valve size */
#define RVALVE     (12) /* Radius of ball in middle of valve */
#define TANK_WALL  (10) /* Thickness of tank wall */

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
#define TANK_HEAT_EXCHANGER   (1)
#define TANK_MANIFOLD_BOTTOM  (2) /* False bottom connecting to TANK_EXIT_BOTTOM */
#define TANK_MANIFOLD_TOP     (4) /* Return manifold at top of tank */
#define TANK_RETURN_BOTTOM    (8) /* Return pipe at bottom of tank */
#define TANK_EXIT_BOTTOM     (16) /* One pipe output at tank bottom */

#define MANUAL_OFF (0)
#define MANUAL_ON  (1)
#define AUTO_OFF   (2)
#define AUTO_ON    (3)

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
#define COLOR_OUT1 QColor(  0,128,255) /* Output pipe color when flow */
#define COLOR_IN0  QColor(102,153,255) /* Input pipe color when no flow */
#define COLOR_IN1  QColor(102,255,255) /* Input pipe color when flow */

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
class Tank : public QGraphicsPolygonItem
{
public:
    Tank(int x, int y, int width, int height, uint8_t options, QString name);
    void setOrientation(int width, int height, uint8_t options);
    void setName(QString name);
    void setValues(qreal temp, qreal sp, qreal vol, qreal power);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF get_coordinate(int which);
protected:
    int       tankWidth, tankHeight;
    QString   tankName;      /* Tank name */
    uint8_t   tankOptions;   /* Tank options: TANK_HEAT_EXCHANGER ... TANK_EXIT_BOTTOM */
    QPolygonF tankPolygon;   /* Contains polygon for drawing the tank */
    qreal     tankTemp;      /* Actual temperature inside tank */
    qreal     tankSetPoint;  /* Setpoint temperature for tank */
    qreal     tankVolume;    /* Actual volume in the tank */
    qreal     tankPower;     /* Actual heating power applied to the tank */
    QPointF   left_pipe1;    /* Coordinate of top-left pipe for connecting a pump */
    QPointF   left_pipe2;    /* Coordinate of bottom-left pipe for connecting a pump */
    QPoint    left_top_pipe; /* Coordinate of top-left pipe for return manifold at top of tank */
    QPoint    bottom_pipe1;  /* Coordinate of bottom pipe for manifold at bottom of tank */
    QPoint    bottom_pipe2;  /* Coordinate of bottom return pipe at bottom of tank */
    QPoint    right_pipe1;   /* Coordinate of top-right pipe for heat-exchanger in tank */
    QPoint    right_pipe2;   /* Coordinate of bottom-right pipe for heat-exchanger in tank */
};

//------------------------------------------------------------------------------------------
class Pipe : public QGraphicsPolygonItem
{
public:
    Pipe(int x, int y, uint8_t type, uint16_t length, QColor color);
    Pipe(QPointF point, uint8_t type, uint16_t length, QColor color);
    void    drawPipe(uint8_t type, uint16_t length, QColor color);
    QPointF get_coordinate(uint8_t side);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void    setColor(QColor color);
protected:
    uint16_t  pipeLength;
    uint8_t   pipeType;
    QPolygonF pipePolygon; /* Contains polygon for drawing the pipe */
    QPointF   left,top,right,bottom; // coordinates of various points
    QColor    pipeColor;
};

//------------------------------------------------------------------------------------------
class Display : public QGraphicsSimpleTextItem
{
public:
    Display(QPointF point);
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF  boundary; // boundary of object
    void    setSubText(QString string);
protected:
    QRectF  boundingRect() const override { return boundary; }
    QString subText;
    int     width;
    int     height;
};

//------------------------------------------------------------------------------------------
class Meter : public QGraphicsSimpleTextItem
{
public:
    Meter(QPointF point, uint8_t type, QString name);
    void    setName(QString name);
    void    setTempValue(qreal value);
    void    setFlowValue(qreal value,qreal temp);
    void    setFlowParameters(uint16_t msec, bool temp_corr, qreal flow_err);
    qreal   getFlowRate(void); // return the flow-rate in L/min.
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void    setError(bool err);
    QPointF get_coordinate(uint8_t side);
    void    initFlowRateDetector(uint8_t perc);
    bool    isFlowRateLow(void);
    QRectF  boundary; // boundary of object

protected:
    QRectF boundingRect() const override { return boundary; }
    QPointF left,top,right,bottom;         // coordinates of various points
    QString meterName;
    qreal   meterValue;
    qreal   meterValueOld;
    qreal   meterdValue;
    uint8_t meterType;
    bool    meterError;
    // Variables for flowrate calculation
    qreal   Ts;              // Time in msec. between two setValue() calls
    bool    tempCorrection;  // true = apply temperature volume correction
    qreal   flowErr;         // error correction percentage for flowmeter
    qreal   flowRate;        // Flow-rate in L/min.
    MA      *pma;            // pointer to moving-average filter for flowrate

    // Variables for flowrate-low detector
    uint8_t frl_std;         // STD state number
    uint8_t frl_tmr;         // Timer value
    double  frl_det_lim;     // Lower-limit for flowrate
    double  frl_min_det_lim; // Minimum flowrate: sensor-check
    uint8_t frl_perc;        // Percentage of max flowrate
};
//------------------------------------------------------------------------------------------


// Base object for Valve and Pump objects
class Base_Valve_Pump : public QGraphicsPolygonItem
{

public:
    Base_Valve_Pump(QPointF point, QString name);
    void    setColor(QColor color);
    void    setName(QString name);
    void    setStatus(uint8_t status);
    uint8_t getStatus(void);
    QPointF get_coordinate(uint8_t side);
    bool    inManualMode(void);

    QRectF  boundary; // boundary of object

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    QString   valveName;                     /* Valve name */
    QColor    valveColor       = Qt::red;    /* color of valve */
    bool      valveOrientation = HORIZONTAL; /* Orientation of valve: HORIZONTAL or VERTICAL */
    QPolygonF valvePolygon;                  /* Contains polygon for drawing the valve */
    uint8_t   valveStatus;                   /* Status of the valve: MANUAL_OFF, MANUAL_ON, AUTO_OFF, AUTO_ON */
    QString   status_text[4] = { "OFF(M)", "ON (M)", "OFF(A)", "ON (A)" };
    QPointF   left,top,right,bottom;         // coordinates of various points

private:
    void      slotAuto(void);
    void      slotManualOff(void);
    void      slotManualOn(void);
    QAction   *setManualOffAction;
    QAction   *setManualOnAction;
    QAction   *setAutoAction;
    QMenu     *contextMenu;
};
//------------------------------------------------------------------------------------------

// Valve object, derived from Base_Valve_Pump
class Valve : public Base_Valve_Pump
{
public:
    Valve(QPointF point, bool orientation, QString name);
    void setOrientation(bool orientation); // draws the object as a polygon

protected:
    QRectF boundingRect() const override { return boundary; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override; // overriding paint()

private:
};
//------------------------------------------------------------------------------------------

// Pump object, derived from Base_Valve_Pump
class Pump : public Base_Valve_Pump
{
public:
    Pump(QPointF point, bool orientation, QString name);
    void setPumpOrientation(bool orientation); // draws the object as a polygon

protected:
    QRectF boundingRect() const override { return boundary; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override; // overriding paint()

private:
};

#endif // HMI_OBJECTS_H
