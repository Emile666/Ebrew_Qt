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

#include "hmi_objects.h"

#include <QBrush>
#include <QTouchEvent>
#include <QtMath>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QTextStream>

//------------------------------------------------------------------------------------------
PowerButton::PowerButton(int x, int y, int width, int height, QString name)
    : QPushButton()
{
    QFont font;

    move(x,y);
    pixmap_off = QPixmap(":/img/button_red_med.png");
    pixmap_on  = QPixmap(":/img/button_green_med.png");
    icon_off   = QIcon(pixmap_off);
    icon_on    = QIcon(pixmap_on);
    buttonState = false;

    font.setPointSize(16);
    font.setBold(true);
    setFont(font);
    setIcon(icon_off);
    setIconSize(QSize(height/2,height/2));
    setFixedSize(QSize(width,height));
    setText(name);
    connect(this,SIGNAL(pressed()),this,SLOT(button_pressed()));
} // PowerButton::PowerButton()

void PowerButton::button_pressed(void)
{
    buttonState = !buttonState;
    if (buttonState)
         setIcon(icon_on);
    else setIcon(icon_off);
} // PowerButton::button_pressed()

bool PowerButton::getButtonState(void)
{
    return buttonState;
} // PowerButton::getButtonState()

void PowerButton::setButtonState(bool state)
{
    buttonState = state;
    if (buttonState)
         setIcon(icon_on);
    else setIcon(icon_off);
} // PowerButton::setButtonState()

//------------------------------------------------------------------------------------------
Tank::Tank(int x, int y, int width, int height, uint8_t options, QString name)
    : QGraphicsPolygonItem()
{
    setPos(x,y);
    setOrientation(width,height,options);
    setName(name);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    //setFlag(QGraphicsItem::ItemIsMovable,true);
} // Tank()

void Tank::setName(QString name)
{
    tankName = name;
} // Tank::setName()

void Tank::setOrientation(int width, int height, uint8_t options)
{
    QPainterPath path;

    tankWidth   = width;
    tankHeight  = height;
    tankOptions = options;

    setBrush(Qt::gray);
    path.moveTo(-width>>1,-height);
    path.lineTo(-width>>1,0);
    path.lineTo(+width>>1,0);
    path.lineTo(+width>>1,-height);
    path.lineTo((+width>>1)-TANK_WALL,-height);
    path.lineTo((+width>>1)-TANK_WALL,-TANK_WALL);
    path.lineTo((-width>>1)+TANK_WALL,-TANK_WALL);
    path.lineTo((-width>>1)+TANK_WALL,-height);
    path.lineTo(-width>>1,-height);
    tankPolygon = path.toFillPolygon();
    setPolygon(tankPolygon);
    // Set coordinates for all input- and output-pipes
    left_pipe1.setX(-45.0-0.5*width);
    left_pipe1.setY(-50.0-0.25*height); // upper-left pipe
    left_pipe2.setX(-45.0-0.5*width);
    left_pipe2.setY(-10.0-0.25*height); // lower-left pipe
    bottom_pipe1.setX(-RPIPE);
    bottom_pipe1.setY(-30);             // bottom pipe connected to manifold
    bottom_pipe2.setX((width>>1)-4*RPIPE);
    bottom_pipe2.setY(-20);             // bottom pipe connected to manifold
    right_pipe1.setX((width>>2)-10);
    right_pipe1.setY(left_pipe1.y());   // upper-right pipe
    right_pipe2.setX((width>>2)-10);
    right_pipe2.setY(left_pipe2.y());   // lower-right pipe
    left_top_pipe.setX(left_pipe1.x());
    left_top_pipe.setY(20-height);      // top-left pipe for return manifold
} // Tank::setOrientation()

void Tank::setValues(qreal temp, qreal sp, qreal vol, qreal power)
{
    tankTemp     = temp;
    tankSetPoint = sp;
    tankVolume   = vol;
    tankPower    = power;
} // Tank::setValues()

QPointF Tank::get_coordinate(int which)
{
    QPointF point = pos();

    switch (which)
    {
        case COORD_LEFT_PIPE1:    point += left_pipe1    + QPoint(0,RPIPE);     break; /* upper-left pipe */
        case COORD_LEFT_PIPE2:    point += left_pipe2    + QPoint(0,RPIPE-1);   break; /* lower-left pipe */
        case COORD_LEFT_TOP_PIPE: point += left_top_pipe + QPoint(0,RPIPE-1);   break; /* for return manifold */
        case COORD_BOTTOM_PIPE1:  point += bottom_pipe1  + QPoint(RPIPE<<1,60); break;
        case COORD_BOTTOM_PIPE2:  point += bottom_pipe2  + QPoint(RPIPE<<1,60); break;
        case COORD_RIGHT_PIPE1:   point += right_pipe1   + QPoint(80,RPIPE-1);  break;
        case COORD_RIGHT_PIPE2:   point += right_pipe2   + QPoint(80,RPIPE-1);  break;
    default:                      point  = QPoint(0,0); break;
    } // which
    return point;
} // Tank::get_coordinate()

void Tank::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainterPath path;
    QFont        font;
    QString      text;
    int          yb;   // Top y-coordinate for value displays

    // Draw tank Title at top of tank
    font.setPointSize(24); // assume 150 width
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(-75,-tankHeight,tankName);

    painter->save();
    if (tankOptions & (TANK_EXIT_BOTTOM | TANK_MANIFOLD_BOTTOM))
    {   // An exit pipe at the bottom of the tank with or without a manifold
        path.addRect(bottom_pipe1.x(),bottom_pipe1.y(),RPIPE<<1,60);
        painter->fillPath(path,COLOR_IN0);
        if (tankOptions & TANK_MANIFOLD_BOTTOM)
        {   // A manifold always has a tank-exit pipe
            painter->setPen(QPen(Qt::blue,5,Qt::SolidLine));
            painter->drawEllipse(1.5*TANK_WALL-(tankWidth>>1),-50,tankWidth-3*TANK_WALL,20);
        } // if
    } // if
    if (tankOptions & TANK_MANIFOLD_TOP)
    {   // A return-manifold at the top of the tank
        path.addRect(left_top_pipe.x(),left_top_pipe.y(),65,RPIPE<<1);
        painter->fillPath(path,COLOR_OUT0);
        painter->setPen(QPen(COLOR_OUT0,5,Qt::SolidLine));
        painter->drawEllipse(1.5*TANK_WALL-(tankWidth>>1),20-tankHeight,tankWidth-3*TANK_WALL,20);
    } // if
    if (tankOptions & TANK_RETURN_BOTTOM)
    {   // A return pipe at the bottom of the tank (no manifold)
        path.addRect(bottom_pipe2.x(),bottom_pipe2.y(),RPIPE<<1,60);
        painter->fillPath(path,COLOR_OUT0);
    } // if
    if (tankOptions & TANK_HEAT_EXCHANGER)
    {   // A heat-exchanger in the middle of the tank
        path.addRect(left_pipe1.x() ,left_pipe1.y() ,90,RPIPE<<1); // upper pipe left
        path.addRect(left_pipe2.x() ,left_pipe2.y() ,90,RPIPE<<1); // lower pipe left
        path.addRect(right_pipe1.x(),right_pipe1.y(),80,RPIPE<<1); // upper pipe right
        path.addRect(right_pipe2.x(),right_pipe2.y(),80,RPIPE<<1); // lower pipe right
        painter->setPen(QPen(COLOR_OUT0,5,Qt::SolidLine));
        int x1 = -tankWidth>>2;
        painter->drawEllipse(x1,left_pipe2.y()+10,tankWidth>>1,10);
        painter->drawEllipse(x1,left_pipe2.y()   ,tankWidth>>1,10);
        painter->drawEllipse(x1,left_pipe2.y()-10,tankWidth>>1,10);
        painter->drawEllipse(x1,left_pipe2.y()-20,tankWidth>>1,10);
        painter->drawEllipse(x1,left_pipe2.y()-30,tankWidth>>1,10);
        painter->drawEllipse(x1,left_pipe2.y()-40,tankWidth>>1,10);
        painter->fillPath(path,COLOR_OUT0);
    } // if
    painter->restore();

    QGraphicsPolygonItem::paint(painter,option,widget);

    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::black);
    font.setPointSize(18);
    font.setBold(true);
    painter->setFont(font);
    //-------------------------------------
    // Temperature with Setpoint Display
    //-------------------------------------
    if (tankOptions & TANK_MANIFOLD_TOP)
    {   // Return manifold in top, this is an MLT
        yb = 100;
    } // if
    else
    {   // No return manifold in top, this is an HLT or boil-kettle
        yb = 20;
    } // else
    painter->drawRect(-(tankWidth>>2),yb+4-tankHeight,120,40); // Draw Temperature display rectangle
    painter->setPen(Qt::red);
    text = QString("%1 °C").arg(tankTemp,2,'f',2);
    painter->drawText(10-(tankWidth>>2),yb+26-tankHeight,text); // Draw Actual temperature
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::yellow);
    text = QString("Setpoint: %1 °C").arg(tankSetPoint,2,'f',1);
    painter->drawText(5-(tankWidth>>2),yb+40-tankHeight,text); // Draw Setpoint temperature
    painter->setPen(Qt::black);
    painter->drawText(5-(tankWidth>>2),yb-tankHeight,"Temperature"); // Draw title
    //-------------------------------------
    // Volume Display
    //-------------------------------------
    yb += 60;
    font.setPointSize(18);
    font.setBold(true);
    painter->setFont(font);
    painter->drawRect(-(tankWidth>>2),yb+4-tankHeight,120,28); // Draw Volume display rectangle
    painter->setPen(Qt::red);
    text = QString("%1 L").arg(tankVolume,2,'f',1);
    painter->drawText(10-(tankWidth>>2),yb+26-tankHeight,text); // Draw Actual Volume
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(5-(tankWidth>>2),yb-tankHeight,"Volume"); // Draw title
    //-------------------------------------
    // Actual Power Display
    //-------------------------------------
    if (!(tankOptions & TANK_MANIFOLD_TOP))
    {   // Only display actual power for a HLT or Boil-kettle
        yb += 50;
        font.setPointSize(18);
        font.setBold(true);
        painter->setFont(font);
        painter->drawRect(-(tankWidth>>2),yb+4-tankHeight,120,28); // Draw Volume display rectangle
        painter->setPen(Qt::red);
        text = QString("%1 %").arg(tankPower,2,'f',0);
        painter->drawText(10-(tankWidth>>2),yb+26-tankHeight,text); // Draw Actual Power
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::black);
        painter->drawText(5-(tankWidth>>2),yb-tankHeight,"Actual Power"); // Draw title
    } // if
} // Tank::paint()

//------------------------------------------------------------------------------------------
Pipe::Pipe(int x, int y, uint8_t type, uint16_t length, QColor color)
    : QGraphicsPolygonItem()
{
    setPos(x,y);
    drawPipe(type,length,color);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    //setFlag(QGraphicsItem::ItemIsMovable,true);
    left = top = right = bottom = QPoint(0,0); // init. all coordinates
} // Pipe()

Pipe::Pipe(QPointF point, uint8_t type, uint16_t length, QColor color)
    : QGraphicsPolygonItem()
{
    setPos(point);
    drawPipe(type,length,color);
} // Pipe()

// This function returns the parent coordinate of top, right, left or bottom of pipe
QPointF Pipe::get_coordinate(uint8_t side)
{
    QPointF point = pos();

    switch (side)
    {
        case COORD_TOP   : point += top;    break; /* top coordinate of pipe */
        case COORD_RIGHT : point += right;  break; /* right coordinate of pipe */
        case COORD_LEFT  : point += left;   break; /* left coordinate of pipe */
        case COORD_BOTTOM: point += bottom; break; /* bottom coordinate of pipe */
    default:               point  = QPoint(0,0); break;
    } // which
    return point;
} // Pipe::get_coordinate()

void Pipe::setColor(QColor color)
{
    pipeColor = color;
    setBrush(color);
} // Pipe:: setFlow()

void Pipe::drawPipe(uint8_t type, uint16_t length, QColor color)
{
    QPainterPath path;

    pipeType   = type;
    pipeLength = length;
    setBrush(color);

    switch (pipeType)
    {
        case PIPE2_LEFT_TOP:
             left = QPoint(-50,0);
             top  = QPoint(0,-50);
             path.moveTo(-50,1-RPIPE);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(-RPIPE,-50);
             path.lineTo(+RPIPE,-50);
             path.lineTo(+RPIPE,RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             break;
        case PIPE2_LEFT_RIGHT:
        case PIPE2_RIGHT_LEFT:
             left  = QPoint(-pipeLength>>1,0);
             right = -left;
             path.moveTo(-pipeLength>>1,1-RPIPE);
             path.lineTo(+pipeLength>>1,1-RPIPE);
             path.lineTo(+pipeLength>>1,+RPIPE);
             path.lineTo(-pipeLength>>1,+RPIPE);
             path.lineTo(-pipeLength>>1,1-RPIPE);
             break;
        case PIPE2_LEFT_BOTTOM:
             left   = QPoint(-50,0);
             bottom = QPoint(0,+50);
             path.moveTo(-50,1-RPIPE);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(+RPIPE,+50);
             path.lineTo(-RPIPE,+50);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             break;
        case PIPE2_TOP_RIGHT:
             top   = QPoint(0,-length);
             right = QPoint(length,0);
             path.moveTo(+RPIPE,-length);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(length,1-RPIPE);
             path.lineTo(length,+RPIPE);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-RPIPE,-length);
             break;
        case PIPE2_TOP_BOTTOM:
        case PIPE2_BOTTOM_TOP:
        case PIPE2_BOTTOM_TOP_NO_ARROW:
             top    = QPoint(0,-(pipeLength>>1));
             bottom = -top;
             path.moveTo(+RPIPE,-(pipeLength>>1));
             path.lineTo(+RPIPE,+pipeLength>>1);
             path.lineTo(-RPIPE,+pipeLength>>1);
             path.lineTo(-RPIPE,-(pipeLength>>1));
             path.lineTo(+RPIPE,-(pipeLength>>1));
             break;
        case PIPE2_BOTTOM_RIGHT:
             bottom = QPoint(0,length);
             right  = QPoint(length,0);
             path.moveTo(-RPIPE,length);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(length,1-RPIPE);
             path.lineTo(length,+RPIPE);
             path.lineTo(+RPIPE,+RPIPE);
             path.lineTo(+RPIPE,length);
             path.lineTo(-RPIPE,length);
             break;
        case PIPE3_NO_TOP:
             right  = QPoint(50,0);
             bottom = QPoint(0,50);
             left   = QPoint(-50,0);
             path.moveTo(-50,1-RPIPE);
             path.lineTo(+50,1-RPIPE);
             path.lineTo(+50,+RPIPE);
             path.lineTo(+RPIPE,+RPIPE);
             path.lineTo(+RPIPE,+50);
             path.lineTo(-RPIPE,+50);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             break;
        case PIPE3_NO_RIGHT:
             top    = QPoint(0,-50);
             bottom = QPoint(0,50);
             left   = QPoint(-50,0);
             path.moveTo(+RPIPE,-50);
             path.lineTo(+RPIPE,+50);
             path.lineTo(-RPIPE,+50);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(-RPIPE,-50);
             path.lineTo(+RPIPE,-50);
             break;
        case PIPE3_NO_BOTTOM:
             right = QPoint(50,0);
             top   = QPoint(0,-50);
             left  = QPoint(-50,0);
             path.moveTo(+50,+RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(-RPIPE,-50);
             path.lineTo(+RPIPE,-50);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(+50,1-RPIPE);
             path.lineTo(+50,+RPIPE);
             break;
        case PIPE3_NO_LEFT:
             right  = QPoint(50,0);
             bottom = QPoint(0,50);
             top    = QPoint(0,-50);
             path.moveTo(-RPIPE,+50);
             path.lineTo(-RPIPE,-50);
             path.lineTo(+RPIPE,-50);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(+50,1-RPIPE);
             path.lineTo(+50,+RPIPE);
             path.lineTo(+RPIPE,+RPIPE);
             path.lineTo(+RPIPE,+50);
             path.lineTo(-RPIPE,+50);
             break;
        case PIPE4_ALL:
             top    = QPoint(0,-50);
             right  = QPoint(50,0);
             bottom = QPoint(0,50);
             left   = QPoint(-50,0);
             path.moveTo(-RPIPE,+50);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-50,+RPIPE);
             path.lineTo(-50,1-RPIPE);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(-RPIPE,-50);
             path.lineTo(+RPIPE,-50);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(+50,1-RPIPE);
             path.lineTo(+50,+RPIPE);
             path.lineTo(+RPIPE,+RPIPE);
             path.lineTo(+RPIPE,+50);
             path.lineTo(-RPIPE,+50);
             break;
    } // switch
    pipePolygon = path.toFillPolygon();
    setPolygon(pipePolygon);
} // Pipe::drawPipe()

void Pipe::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainterPath path;
    QPen pen;

    pen.setColor(Qt::black);
    pen.setWidth(3);
    painter->setPen(pen);
    if (pipeType == PIPE2_LEFT_RIGHT)
    {
        painter->drawLine(bottom + QPoint(-20,20), bottom + QPoint(10,20));
        painter->drawLine(bottom + QPoint(10,20) , bottom + QPoint( 5,15));
        painter->drawLine(bottom + QPoint( 5,15) , bottom + QPoint( 5,25));
        painter->drawLine(bottom + QPoint( 5,25) , bottom + QPoint(10,20));
    } // if
    else if (pipeType == PIPE2_RIGHT_LEFT)
    {
        painter->drawLine(bottom + QPoint(20,20) , bottom + QPoint(-10,20));
        painter->drawLine(bottom + QPoint(-10,20), bottom + QPoint(-5,15));
        painter->drawLine(bottom + QPoint(- 5,15), bottom + QPoint(-5,25));
        painter->drawLine(bottom + QPoint(- 5,25), bottom + QPoint(-10,20));
    } // if
    else if (pipeType == PIPE2_TOP_BOTTOM)
    {
        painter->drawLine(right + QPoint(20,-10), right + QPoint(20,20));
        painter->drawLine(right + QPoint(20,20) , right + QPoint(25,15));
        painter->drawLine(right + QPoint(25,15) , right + QPoint(15,15));
        painter->drawLine(right + QPoint(15,15) , right + QPoint(20,20));
    } // if
    else if (pipeType == PIPE2_BOTTOM_TOP)
    {
        painter->drawLine(right + QPoint(20,+20), right + QPoint(20,-10));
        painter->drawLine(right + QPoint(20,-10), right + QPoint(25,-5));
        painter->drawLine(right + QPoint(25,-5) , right + QPoint(15,-5));
        painter->drawLine(right + QPoint(15,-5) , right + QPoint(20,-10));
    } // if
    QGraphicsPolygonItem::paint(painter,option,widget);
} // Pipe::paint()
//------------------------------------------------------------------------------------------

Display::Display(QPointF point)
    : QGraphicsSimpleTextItem()
{
    QFont font;

    width  = 600;
    height = 45;
    font.setPointSize(18);
    setFont(font);
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::red));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setPos(point);
    boundary = QRectF(-5,-20,width,height+20);
} // Display::Display()

void Display::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QFont font;

    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::black);
    painter->drawRect(-5,0,width,height); // Draw display rectangle
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::yellow);
    painter->drawText(5,41,subText); // Draw sub-text
    painter->setPen(Qt::black);
    painter->drawText(0,-5,"Current STD state"); // Draw title
    QGraphicsSimpleTextItem::paint(painter,option,widget);
} // Display::paint()

void Display::setSubText(QString string)
{
    subText = string;
} // Display::setSubText()

//------------------------------------------------------------------------------------------
Meter::Meter(QPointF point, uint8_t type, QString name)
    : QGraphicsSimpleTextItem()
{
    QFont font;
    font.setStyleHint(QFont::Times, QFont::PreferAntialias);
    font.setBold(true);
    font.setPointSize(28);

    setBrush(Qt::black);
    setFont(font);
    setPos(point);
    setName(name);
    setTempValue(0.0); // Init. meterValue
    setError(false);   // No error => green colour
    meterType = type;
    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        MA *p = new MA(METER_FLOW_MA_N,0.0);
        pma   = p; // save pointer
        setText("F");
    } // if
    else setText("T");
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    if ((meterType == METER_HFLOW) || (meterType == METER_HTEMP))
         boundary = QRectF(-14,-15,50,80);  // horizontal lay-out
    else boundary = QRectF(-35,-10,160,60); // vertical lay-out
    left     = QPointF(-13,+24);
    right    = QPointF(+34,+24);
    top      = QPointF(+11,0);
    bottom   = QPointF(+11,48);
} // Meter::Meter()

void Meter::setTempValue(qreal value)
{
    meterValue = value;
} // Meter::setValue()

void Meter::setFlowParameters(uint16_t msec, bool temp_corr, qreal flow_err)
{
    Ts             = msec;
    if (Ts < 100) Ts = 100; // min. of 100 msec.
    tempCorrection = temp_corr;
    flowErr        = flow_err;
} // Meter::setFlowParameters()

void Meter::setFlowValue(qreal value,qreal temp)
{
    meterValue  = value;
    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        if (tempCorrection)
            meterValue /= (1.0 + 0.00021 * (temp - 20.0));
        // Apply Calibration error correction
        meterValue *= 1.0 + 0.01 * flowErr;
        // Calculate Flow-rate in L per minute: Ts [msec.]
        flowRate = (60000.0 / Ts) * (meterValue - meterValueOld);
        meterValueOld = meterValue;
        flowRate = pma->moving_average(flowRate);
    } // if
} // Meter::setValue()

qreal Meter::getFlowRate(void)
{
    return flowRate;
} // Meter::getFlowRate()

void Meter::setName(QString name)
{
    meterName = name;
}

void Meter::setError(bool err)
{
    meterError = err;
}

QPointF Meter::get_coordinate(uint8_t side)
{
    QPointF point = pos();

    switch (side)
    {
        case COORD_TOP   : point += top;    break; /* top coordinate of pipe */
        case COORD_RIGHT : point += right;  break; /* right coordinate of pipe */
        case COORD_LEFT  : point += left;   break; /* left coordinate of pipe */
        case COORD_BOTTOM: point += bottom; break; /* bottom coordinate of pipe */
        default:           point  = QPoint(0,0); break;
    } // which
    return point;
} // Pipe::get_coordinate()

void Meter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainterPath path;
    QFont        font;
    QString      text1,text2 = "";

    path.addEllipse(QRect(-14,-1,50,50));
    if (meterError)
         painter->fillPath(path,Qt::red);
    else painter->fillPath(path,Qt::green);
    painter->drawArc(QRect(-14,-1,50,50),0,5760);
    font.setPointSize(12);
    font.setBold(true);
    painter->setFont(font);

    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        text1 = QString("%1 L").arg(meterValue,1,'f',1);
        text2 = QString("%1 L/min.").arg(meterdValue,1,'f',1);
    } // if
    else
    {
        text1 = QString("%1 °C").arg(meterValue);
    } // else
    if ((meterType == METER_HFLOW) || (meterType == METER_HTEMP))
    {    // horizontal lay-out
         painter->drawText(-14,62,text1);
         if (meterdValue > 0.0) painter->drawText(-14,77,text2);
    } // if
    else
    {   // vertical lay-out
        painter->drawText(+40,25,text1);
        if (meterdValue) painter->drawText(+40,40,text2);
    } // else
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    if ((meterType == METER_HFLOW) || (meterType == METER_HTEMP))
         painter->drawText(-10,-4,meterName);
    else painter->drawText(-37,+2,meterName);
    QGraphicsSimpleTextItem::paint(painter,option,widget);
} // Meter::paint()

/*------------------------------------------------------------------
  Purpose  : This function initialises the flowrate-low detector.
             It needs to be called before calling the isFlowRateLow()
             function.
  Variables: perc: the percentage of the nominal detected flowrate that
                   is used for detection of low flowrate
  Returns  : -
  ------------------------------------------------------------------*/
void Meter::initFlowRateDetector(uint8_t perc)
{
    frl_std         = 0;    // set state nr to 0
    frl_tmr         = 0;    // set timer to 0
    frl_min_det_lim = 1.5;  // Flow-sensor should at least give 1.5 L/min.
    frl_det_lim     = 0.0;  // set detection limit to 0
    frl_perc        = perc; // init. percentage value
} // Meter::initFlowRateDetector()

/*------------------------------------------------------------------
  Purpose  : This function is used to detect a low flowrate. First,
             the nominal flowrate is determined. If the flowrate drops
             below a percentage of this nominal flowrate, the flowrate
             is considered low and true is returned.
  Variables:
   flowRate: the flowrate in L/min. that is used for detection
  Returns  : true: flowrate is low, false: flow-rate is still high
  ------------------------------------------------------------------*/
bool Meter::isFlowRateLow(void)
{
    bool retv = false; // return-value

    switch (frl_std)
    {
        case 0 : // Give flow-rate time to increase because of the MA-filter
            // Use 30 seconds which is enough to stabilize the MA-filter
            if (++frl_tmr > 30)
            {
                frl_det_lim = 0.0; // reset detection-limit
                frl_tmr     = 0;   // reset timer
                frl_std     = 1;   // goto next state
            } // if
            break;
        case 1 : // Calculate the average flow-rate for the next 30 seconds
            // Use perc of average flow-rate as detection-limit
            if (++frl_tmr > 30)
            {
                frl_det_lim /= 30.0; // calculate average flow-rate
                frl_std      = 2;    // goto next state
            } // if
            else frl_det_lim += flowRate; // calculate average flowrate
            break;
        case 2:  // Now check if average flow-rate is above a minimum. If not, repeat calculations
            if (frl_det_lim > frl_min_det_lim)
            {    // average flow-rate > minimum flow-rate?
                frl_det_lim *= frl_perc; // Percentage of average flowrate
                frl_det_lim *= 0.01;     // Divide by 100%
                frl_std = 3;
            } // if
            else
            {
                frl_tmr = 0; // reset timer
                frl_std = 0; // repeat measurements
            } // else
            break;
        case 3:  // Now check if flow-rate decreases
            if (flowRate < frl_det_lim) // flow-rate < percentage of average flow-rate?
                retv = true;
            break;
        default: frl_std = 0;
                 frl_tmr = 0;
                 break;
        } // switch
    return retv;
} // Meter::isFlowRateLow()

//------------------------------------------------------------------------------------------
Base_Valve_Pump::Base_Valve_Pump(QPointF point, QString name)
    : QGraphicsPolygonItem()
{
    setPos(point);
    setColor(Qt::red);
    setName(name);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    //setFlag(QGraphicsItem::ItemIsMovable,true);
    left = top = right = bottom = QPoint(0,0); // init. all coordinates
} // Base_Valve_Pump()

// This function returns the parent coordinate of top, right, left or bottom of pipe
QPointF Base_Valve_Pump::get_coordinate(uint8_t side)
{
    QPointF point = pos();

    switch (side)
    {
        case COORD_TOP   : point += top;    break; /* top coordinate of pipe */
        case COORD_RIGHT : point += right;  break; /* right coordinate of pipe */
        case COORD_LEFT  : point += left;   break; /* left coordinate of pipe */
        case COORD_BOTTOM: point += bottom; break; /* bottom coordinate of pipe */
    default:               point  = QPoint(0,0); break;
    } // which
    return point;
} // Base_Valve_Pump::get_coordinate()

void Base_Valve_Pump::setColor(QColor color)
{
    valveColor = color;
    setBrush(valveColor);
} // setRadius()

void Base_Valve_Pump::setName(QString name)
{
    valveName = name;
}

void Base_Valve_Pump::setStatus(uint8_t status)
{
    valveStatus = status;
    if ((valveStatus == MANUAL_OFF) || (valveStatus == AUTO_OFF))
         setColor(Qt::red);
    else setColor(Qt::green);
}

bool Base_Valve_Pump::inManualMode(void)
{
    if ((valveStatus == MANUAL_OFF) || (valveStatus == MANUAL_ON))
         return true;
    else return false;
} // Base_Valve_Pump::inManualMode()

uint8_t Base_Valve_Pump::getStatus(void)
{
    return valveStatus;
} // Base_Valve_Pump::getStatus()

void Base_Valve_Pump::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        mouseEvent->accept();
        if (++valveStatus > AUTO_ON) valveStatus = MANUAL_OFF;
        setStatus(valveStatus);
    }
    else if (mouseEvent->button() == Qt::RightButton)
    {
        mouseEvent->accept();
        setAutoAction = new QAction("Auto");
        //connect(setAutoAction,SIGNAL(triggered()),this,SLOT(slotAuto()));
        setManualOffAction = new QAction("OFF (M)");
        //QObject::connect(setManualOffAction,SIGNAL(triggered()),0,SLOT(slotManualOff()));
        setManualOnAction = new QAction("ON (M)");
        //QObject::connect(setManualOnAction,SIGNAL(triggered()),0,SLOT(slotManualOn()));
        contextMenu = new QMenu();
        contextMenu->addAction(setAutoAction);
        contextMenu->addAction(setManualOffAction);
        contextMenu->addAction(setManualOnAction);
        contextMenu->exec(QCursor::pos());
        delete contextMenu;
        contextMenu = NULL;
    } // if
    QGraphicsPolygonItem::mousePressEvent(mouseEvent);
}

void Base_Valve_Pump::slotManualOff(void)
{
    setStatus(MANUAL_OFF);
}

void Base_Valve_Pump::slotManualOn(void)
{
   setStatus(MANUAL_ON);
}

void Base_Valve_Pump::slotAuto(void)
{
    setStatus(AUTO_OFF);
}

//------------------------------------------------------------------------------------------
Valve::Valve(QPointF point, bool orientation, QString name)
    : Base_Valve_Pump(point,name)
{
    setOrientation(orientation); // Draw the valve
    valveStatus = AUTO_OFF;
} // Valve::Valve()

void Valve::setOrientation(bool orientation)
{
    QPainterPath path;
    float x1 = qSqrt(0.5)*RVALVE;

    valveOrientation = orientation;
    path.moveTo(-VALVE_SIZE>>1,-VALVE_SIZE>>1);
    path.lineTo(-x1,-x1);
    if (orientation == HORIZONTAL)
    {
        path.arcTo(-RVALVE,-RVALVE,RVALVE<<1,RVALVE<<1,135,-90);
        path.lineTo(VALVE_SIZE>>1, -VALVE_SIZE>>1);
        path.lineTo(VALVE_SIZE>>1, +VALVE_SIZE>>1);
        path.lineTo(x1,x1);
        path.arcTo(-RVALVE,-RVALVE,RVALVE<<1,RVALVE<<1,315,-90);
        path.lineTo(-VALVE_SIZE>>1, +VALVE_SIZE>>1);
        path.lineTo(-VALVE_SIZE>>1, -VALVE_SIZE>>1);
        left  = QPoint(-VALVE_SIZE>>1,0);
        right = -left;
        // set boundary with enough room for drawText() in paint()
        boundary = QRectF(-10-(VALVE_SIZE>>1),-10-(VALVE_SIZE>>1),20+VALVE_SIZE,35+VALVE_SIZE);
    }
    else
    {   // VERTICAL
        path.arcTo(-RVALVE,-RVALVE,RVALVE<<1,RVALVE<<1,135,+90);
        path.lineTo(-VALVE_SIZE>>1, +VALVE_SIZE>>1);
        path.lineTo(+VALVE_SIZE>>1, +VALVE_SIZE>>1);
        path.lineTo(x1,x1);
        path.arcTo(-RVALVE,-RVALVE,RVALVE<<1,RVALVE<<1,-45,+90);
        path.lineTo(+VALVE_SIZE>>1, -VALVE_SIZE>>1);
        path.lineTo(-VALVE_SIZE>>1, -VALVE_SIZE>>1);
        bottom = QPoint(0,VALVE_SIZE>>1);
        top    = -bottom;
        // set boundary with enough room for drawText() in paint()
        boundary = QRectF(-60-(VALVE_SIZE>>1),2-(VALVE_SIZE>>1),60+VALVE_SIZE,0+VALVE_SIZE); // extra space left for title and status
    } // else
    valvePolygon = path.toFillPolygon();
    setPolygon(valvePolygon);
} // Valve::SetValveOrientation()

void Valve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QFont font;
    font.setStyleHint(QFont::Times, QFont::PreferAntialias);
    font.setBold(true);
    font.setPointSize(20);
    painter->setFont(font);
    painter->save();
    if (valveOrientation == HORIZONTAL)
    {
        painter->drawText(-0.35*VALVE_SIZE,-1.5*RVALVE,valveName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-0.5*VALVE_SIZE,+0.8*VALVE_SIZE,status_text[valveStatus]);
    } // if
    else
    {   // VERTICAL
        painter->drawText(-1.1*VALVE_SIZE,5-1.2*RVALVE,valveName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-1.4*VALVE_SIZE,5+0.7*RVALVE,status_text[valveStatus]);
    } // else
    painter->restore();
    QGraphicsPolygonItem::paint(painter,option,widget);
} //

//------------------------------------------------------------------------------------------
Pump::Pump(QPointF point, bool orientation, QString name)
    : Base_Valve_Pump(point,name)
{
    setPumpOrientation(orientation); // Draw the valve
    valveStatus = AUTO_OFF;
} // Pump()

void Pump::setPumpOrientation(bool orientation)
{
    QPainterPath path;
    double x1 = qSqrt(RPUMP*RPUMP - RPIPE*RPIPE); // x-coord of pipe-in with PUMP at left side
    double x1a = 180 - qAtan2(RPIPE,x1) * 180 / M_PI;
    double x2 = qSqrt(10*RPUMP - 25); // x-coord at y=25: pipe-out with PUMP at right side
    double x2a = qAtan2(RPUMP-5,x2) * 180 / M_PI;
    double y3 = RPUMP-5-2*RPIPE;
    double x3 = qSqrt(RPUMP*RPUMP-(5-RPUMP+2*RPIPE)*(5-RPUMP+2*RPIPE));
    double x3a = qAtan2(y3,x3) * 180 / M_PI;

    valveOrientation = orientation;
    if (orientation == OUT_RIGHT)
    {   // Pump output is on the right
        path.moveTo(-60,-RPIPE);
        path.lineTo(-x1,-RPIPE);
        path.arcTo(-RPUMP,-RPUMP,2*RPUMP,2*RPUMP,x1a,x2a-x1a);
        path.lineTo(+60,5-RPUMP);
        path.lineTo(+60,-y3);
        path.lineTo(x3,5-RPUMP+2*RPIPE); // 30 graden
        path.arcTo(-RPUMP,-RPUMP,2*RPUMP,2*RPUMP,x3a,-45-x3a);
        path.lineTo(RPUMP+5,RPUMP+5);
        path.lineTo(-RPUMP-5,RPUMP+5);
        path.lineTo(-RPUMP*M_SQRT1_2,RPUMP*M_SQRT1_2);
        path.arcTo(-RPUMP,-RPUMP,2*RPUMP,2*RPUMP,225,135-x1a);
        path.lineTo(-60,RPIPE);
        path.lineTo(-60,-RPIPE);
        left  = QPoint(-60,0);
        right = -left;
        boundary = QRectF(-60,-(RPUMP<<1),117,RPUMP<<2); // extra space on top and bottom for title and status
    }
    else
    {   // Pump output is on the left
    } // else
    valvePolygon = path.toFillPolygon();
    setPolygon(valvePolygon);
} // SetPumpOrientation()

void Pump::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QFont font;
    QPen  pen;

    font.setStyleHint(QFont::Times, QFont::PreferAntialias);
    font.setBold(true);
    font.setPointSize(20);
    pen.setWidth(3);

    painter->setFont(font);
    painter->save();
    if (valveOrientation == OUT_RIGHT)
    {
        painter->drawText(-20,-40,valveName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-35,RPUMP+25,status_text[valveStatus]);
        font.setPointSize(10);
        painter->setFont(font);
        painter->drawText(-50,22,"in");
        painter->drawText(38,+3,"out");
        painter->setBrush(Qt::white);
    }
    painter->restore();
    QGraphicsPolygonItem::paint(painter,option,widget);
    painter->setPen(pen);
    painter->drawArc(-10,-10,20,20,0,5760);
} // paint()


