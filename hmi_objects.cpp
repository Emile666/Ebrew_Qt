/**************************************************************************************
** Filename    : hmi_objects.cpp
** Author      : Emile
** Purpose     : This file contains all graphical objects that are used by MainEbrew:
**               PowerButton: A push-button with a green/red LED in it.
**               Meter      : A flowmeter or temperaturesensor, showing actual values.
**               Tank       : A tank object for constructing a HLT, MLT or Boil-kettle.
**               Pipe       : A pipe which is used to connect everything. A pipe can have
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
#include "hmi_objects.h"

#include <QBrush>
#include <QTouchEvent>
#include <QtMath>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QTextStream>

//------------------------------------------------------------------------------------------
// Pushbutton with a green/red LED
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
// Tank object for HLT, MLT and Boil-kettle
//------------------------------------------------------------------------------------------
Tank::Tank(int x, int y, int width, int height, uint16_t options)
    : QGraphicsPolygonItem()
{
    setPos(x,y);
    setOrientation(width,height,options);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
} // Tank()

void Tank::setNameVolume(QString name, qreal minVol)
{
    tankName      = name;
    tankMinVolume = minVol;
} // Tank::setNameVolume()

uint8_t Tank::getHeatingOptions(void)
{   // Energy-Sources start at TANK_GAS_MODU (0x0100) in tankOptions
    uint8_t retv = (uint8_t)((tankOptions >> 8) & 0x001F);
    return retv;
} // getHeatingOptions()

void Tank::setHeatingOptions(uint8_t options)
{   // Energy-Sources start at TANK_GAS_MODU (0x0100) in tankOptions
    tankOptions &= ~TANK_HEAT_SOURCES; // clear all heat-source bits
    tankOptions |= (((uint16_t)options & 0x001F) << 8);
} // setHeatingOptions()

void Tank::clrHeatingOptions(uint8_t options)
{   // Energy-Sources start at TANK_GAS_MODU (0x0100) in tankOptions
    tankOptions &= (~((uint16_t)options & 0x001F) << 8);
} // clrHeatingOptions()

void Tank::setOrientation(int width, int height, uint16_t options)
{
    QPainterPath path;

    tankWidth   = width;
    tankHeight  = height;
    tankOptions = options;

    // Draw the walls of the tank with a thickness of TANK_WALL
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
    leftPipe1.setX(-10.0-0.5*width);
    leftPipe1.setY(-50.0-0.25*height); // upper-left pipe
    leftPipe2.setX(-10.0-0.5*width);
    leftPipe2.setY(-10.0-0.25*height); // lower-left pipe
    bottomPipe1.setX(-RPIPE);
    bottomPipe1.setY(-30);             // bottom pipe connected to manifold
    bottomPipe2.setX((width>>1)-4*RPIPE);
    bottomPipe2.setY(-20);             // bottom pipe return pipe
    rightPipe1.setX(0.25*width-10.0);
    rightPipe1.setY(leftPipe1.y());    // upper-right pipe
    rightPipe2.setX(rightPipe1.x());
    rightPipe2.setY(leftPipe2.y());    // lower-right pipe
    leftTopPipe.setX(leftPipe1.x()+TANK_WALL);
    leftTopPipe.setY(20-height);       // top-left pipe for return manifold
    // Determine proper boundary for tank object
    qreal x,w,h;
    if (options & (TANK_MANIFOLD_TOP | TANK_HEAT_EXCHANGER))
         x = leftPipe1.x(); // there are pipes on the left
    else x = -1.0-0.5*width;    // no pipes on the left
    if (options & TANK_HEAT_EXCHANGER)
         w = 0.25*width + 70 - x; // pipes on the right
    else w = 0.50*width - x;
    if (options & (TANK_EXIT_BOTTOM | TANK_MANIFOLD_BOTTOM | TANK_RETURN_BOTTOM))
         h = height + 30;
    else h = height;
    boundary = QRectF(x,-height-25,w,h+25);
    colLeftPipes   = COLOR_IN0;        // connected to second pump
    colBottomPipe1 = COLOR_IN0;        // tank-output, connected to pump-input
    colBottomPipe2 = COLOR_OUT0;       // tank-input, connected to pump-output
    colRightPipes  = COLOR_OUT0;       // heat-exchanger pipes, connected to pump-output
    colTopPipe     = COLOR_OUT0;       // tank top-return pipe, connected to pump-output
} // Tank::setOrientation()

void Tank::setColor(uint8_t pipe, QColor color)
{
    switch (pipe)
    {
        case COLOR_LEFT_PIPES  : colLeftPipes   = color; break;
        case COLOR_BOTTOM_PIPE1: colBottomPipe1 = color; break;
        case COLOR_BOTTOM_PIPE2: colBottomPipe2 = color; break;
        case COLOR_RIGHT_PIPES : colRightPipes  = color; break;
        case COLOR_TOP_PIPE    : colTopPipe     = color; break;
        default: break;
    } // switch
} // Tank::setColor()

void Tank::setValues(qreal temp, qreal sp, qreal vol, qreal power)
{
    if (temp > SENSOR_VAL_LIM_OK)
    {   // valid
        tankTemp    = temp;
        tankTempErr = false;
    } // if
    else
    {   // faulty or not connected
        tankTemp    = 0.00;
        tankTempErr = true;
    } // else
    tankSetPoint = sp;
    tankVolume   = vol;
    // Disable electrical heating when water volume is too low and the heaters are exposed.
    if (tankVolume < tankMinVolume) tankOptions &= ~(TANK_ELEC_HEATER1 | TANK_ELEC_HEATER2 | TANK_ELEC_HEATER3);
    tankPower    = power;
} // Tank::setValues()

QPointF Tank::getCoordinates(int which)
{
    QPointF point = pos();

    switch (which)
    {
        case COORD_LEFT_PIPE1:    point += leftPipe1    + QPointF(0,RPIPE);    break; /* upper-left pipe */
        case COORD_LEFT_PIPE2:    point += leftPipe2    + QPointF(0,RPIPE);    break; /* lower-left pipe */
        case COORD_LEFT_TOP_PIPE: point += leftTopPipe  + QPointF(0,RPIPE);    break; /* for return manifold */
        case COORD_BOTTOM_PIPE1:  point += bottomPipe1  + QPointF(RPIPE,60);   break;
        case COORD_BOTTOM_PIPE2:  point += bottomPipe2  + QPointF(RPIPE,50);   break;
        case COORD_RIGHT_PIPE1:   point += rightPipe1   + QPointF(80,RPIPE-1); break;
        case COORD_RIGHT_PIPE2:   point += rightPipe2   + QPointF(80,RPIPE);   break;
    default:                      point  = QPoint(0,0); break;
    } // which
    return point;
} // Tank::getCoordinates()

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
    painter->setPen(QPen(Qt::blue));
    painter->drawText(-75,-tankHeight,tankName);
    // NOTE Use QLinearGradient color blue-white-color when pipe is ON
    if (tankOptions & (TANK_EXIT_BOTTOM | TANK_MANIFOLD_BOTTOM))
    {   // An exit pipe at the bottom of the tank with or without a manifold
        painter->fillRect(bottomPipe1.x(),bottomPipe1.y(),RPIPE<<1,60,colBottomPipe1);
        if (tankOptions & TANK_MANIFOLD_BOTTOM)
        {   // A manifold always has a tank-exit pipe
            painter->setPen(QPen(colBottomPipe1,5,Qt::SolidLine));
            painter->drawEllipse(1.5*TANK_WALL-(tankWidth>>1),-50,tankWidth-3*TANK_WALL,20);
        } // if
    } // if
    if (tankOptions & TANK_MANIFOLD_TOP)
    {   // A return-manifold at the top of the tank
        painter->fillRect(leftTopPipe.x(),leftTopPipe.y(),50,2.0*RPIPE+1,colTopPipe);
        painter->setPen(QPen(colTopPipe,5,Qt::SolidLine));
        painter->drawEllipse(1.5*TANK_WALL-(tankWidth>>1),20-tankHeight,tankWidth-3*TANK_WALL,20);
    } // if
    if (tankOptions & TANK_RETURN_BOTTOM)
    {   // A return pipe at the bottom of the tank (no manifold)
        painter->fillRect(bottomPipe2.x(),bottomPipe2.y(),RPIPE<<1,50,colBottomPipe2);
    } // if
    if (tankOptions & TANK_HEAT_EXCHANGER)
    {   // A heat-exchanger in the middle of the tank
        painter->fillRect(leftPipe1.x() ,leftPipe1.y() ,90,RPIPE<<1,colLeftPipes); // upper pipe left
        painter->fillRect(leftPipe2.x() ,leftPipe2.y() ,90,RPIPE<<1,colLeftPipes); // lower pipe left

        painter->fillRect(rightPipe1.x(),rightPipe1.y(),80,RPIPE<<1,colRightPipes); // upper pipe right
        painter->fillRect(rightPipe2.x(),rightPipe2.y(),80,RPIPE<<1,colRightPipes); // lower pipe right
        int x1 = -tankWidth>>2;
        painter->setPen(QPen(colRightPipes,5,Qt::SolidLine));
        painter->drawEllipse(x1,leftPipe2.y()+10,tankWidth>>1,10);
        painter->drawEllipse(x1,leftPipe2.y()   ,tankWidth>>1,10);
        painter->drawEllipse(x1,leftPipe2.y()-10,tankWidth>>1,10);
        painter->drawEllipse(x1,leftPipe2.y()-20,tankWidth>>1,10);
        painter->drawEllipse(x1,leftPipe2.y()-30,tankWidth>>1,10);
        painter->drawEllipse(x1,leftPipe2.y()-40,tankWidth>>1,10);
        painter->fillPath(path,colRightPipes);
    } // if

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
    {   // No return manifold in top, this is an HLT or Boil-kettle
        yb = 20;
    } // else
    int xbase = -10-(tankWidth>>2); // top-left coord. of temperature display
    painter->drawRect(xbase,yb+4-tankHeight,130,40); // Draw Temperature display rectangle
    painter->setPen(Qt::red);
    text = QString("%1 °C").arg(tankTemp,2,'f',tankTemp < 100.0 ? 2 : 1); // 1 decimal for > 99.99 °C
    painter->drawText(xbase+10,yb+26-tankHeight,text); // Draw Actual temperature
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::yellow);
    text = QString("Setpoint: %1 °C").arg(tankSetPoint,2,'f',1);
    painter->drawText(xbase+5,yb+40-tankHeight,text); // Draw Setpoint temperature
    painter->setPen(Qt::black);
    painter->drawText(xbase+5,yb-tankHeight,"Temperature"); // Draw title

    painter->setPen(Qt::black);
    if (tankTempErr)
         painter->setBrush(Qt::red);
    else painter->setBrush(Qt::green);
    QPoint point(145-(tankWidth>>2),yb+25-tankHeight); // coord. for pseudo temp. meter
    font.setPointSize(20);
    font.setBold(true);
    painter->setFont(font);
    painter->drawEllipse(point,20,20);
    point += QPoint(-7,10);
    painter->drawText(point,"T");
    painter->setBrush(Qt::lightGray);

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
    painter->drawText(5-(tankWidth>>2),yb-tankHeight,"Actual Volume"); // Draw title

    //-------------------------------------
    // Actual Power Display
    //-------------------------------------
    if (!(tankOptions & TANK_MANIFOLD_TOP))
    {   // Only display actual power for a HLT or Boil-kettle
        yb += 50;
        font.setPointSize(18);
        font.setBold(true);
        painter->setFont(font);
        painter->drawRect(-(tankWidth>>2),yb+4-tankHeight,120,28); // Draw Power display rectangle
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

void Tank::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (tankOptions & TANK_CONTEXTMENU)
    {
        QMenu menu;
        QAction *enaGas   = menu.addAction("Modulating Gas-burner");
        QAction *enaGasNM = menu.addAction("Non-modulating Gas-burner");
        QAction *enaEle1  = menu.addAction("Electric Heater 1");
        QAction *enaEle2  = menu.addAction("Electric Heater 2");
        QAction *enaEle3  = menu.addAction("Electric Heater 3");
        enaGas->setCheckable(true);
        enaGasNM->setCheckable(true);
        enaEle1->setCheckable(true);
        enaEle2->setCheckable(true);
        enaEle3->setCheckable(true);
        enaGas->setChecked(tankOptions & TANK_GAS_MODU);
        enaGasNM->setChecked(tankOptions & TANK_GAS_NON_MODU);
        enaEle1->setChecked(tankOptions & TANK_ELEC_HEATER1);
        enaEle2->setChecked(tankOptions & TANK_ELEC_HEATER2);
        enaEle3->setChecked(tankOptions & TANK_ELEC_HEATER3);
        QAction *selectedAction = menu.exec(event->screenPos());
        if (selectedAction == enaGas)
        {
            if (enaGas->isChecked())
                 tankOptions |=  TANK_GAS_MODU;
            else tankOptions &= ~TANK_GAS_MODU;
            enaGas->setChecked(tankOptions & TANK_GAS_MODU);
        } // if
        if (selectedAction == enaGasNM)
        {
            if (enaGasNM->isChecked())
                 tankOptions |=  TANK_GAS_NON_MODU;
            else tankOptions &= ~TANK_GAS_NON_MODU;
            enaGasNM->setChecked(tankOptions & TANK_GAS_NON_MODU);
        } // if
        if (selectedAction == enaEle1)
        {
            if (enaEle1->isChecked())
                 tankOptions |=  TANK_ELEC_HEATER1;
            else tankOptions &= ~TANK_ELEC_HEATER1;
           enaEle1->setChecked(tankOptions & TANK_ELEC_HEATER1);
         } // if
        if (selectedAction == enaEle2)
        {
            if (enaEle2->isChecked())
                 tankOptions |=  TANK_ELEC_HEATER2;
            else tankOptions &= ~TANK_ELEC_HEATER2;
           enaEle2->setChecked(tankOptions & TANK_ELEC_HEATER2);
         } // if
        if (selectedAction == enaEle3)
        {
            if (enaEle3->isChecked())
                 tankOptions |=  TANK_ELEC_HEATER3;
            else tankOptions &= ~TANK_ELEC_HEATER3;
           enaEle3->setChecked(tankOptions & TANK_ELEC_HEATER3);
         } // if
    } // if
} // Tank::contextMenuEvent()

//------------------------------------------------------------------------------------------
// Pipe object
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
    setColor(color);
    drawPipe(type,length,color);
} // Pipe()

// This function returns the parent coordinate of top, right, left or bottom of pipe
QPointF Pipe::getCoordinates(uint8_t side)
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
} // Pipe::getCoordinates()

void Pipe::setColor(QColor color)
{
    pipeColor = color;
    setBrush(color);
} // Pipe:: setFlow()

void Pipe::drawPipe(uint8_t type, uint16_t length, QColor color)
{
    QPainterPath path;
    int          x;

    pipeType   = type;
    pipeLength = length;
    setBrush(color);

    switch (pipeType)
    {
        case PIPE2_LEFT_TOP: // From lower-left to top-right
             left = QPoint(-length,0);
             top  = QPoint(0,-length);
             path.moveTo(-length,1-RPIPE);
             path.lineTo(-RPIPE,1-RPIPE);
             path.lineTo(-RPIPE,-length);
             path.lineTo(+RPIPE,-length);
             path.lineTo(+RPIPE,RPIPE);
             path.lineTo(-length,+RPIPE);
             path.lineTo(-length,1-RPIPE);
             boundary = QRectF(-length,-length,length+RPIPE,length+RPIPE);
             break;
        case PIPE2_LEFT_RIGHT: // From left to right
        case PIPE2_RIGHT_LEFT: // From right to left
             x = pipeLength >> 1;
             left  = QPoint(-x,0);
             right = -left;
             path.moveTo(-x,1-RPIPE);
             path.lineTo(+x,1-RPIPE);
             path.lineTo(+x,+RPIPE);
             path.lineTo(-x,+RPIPE);
             path.lineTo(-x,1-RPIPE);
             boundary = QRectF(-x,-RPIPE,pipeLength,2*RPIPE+2);
             break;
        case PIPE2_LEFT_BOTTOM:
             left   = QPoint(-length,0);
             bottom = QPoint(0,+length);
             path.moveTo(-length,1-RPIPE);
             path.lineTo(+RPIPE,1-RPIPE);
             path.lineTo(+RPIPE,+length);
             path.lineTo(-RPIPE,+length);
             path.lineTo(-RPIPE,+RPIPE);
             path.lineTo(-length,+RPIPE);
             path.lineTo(-length,1-RPIPE);
             boundary = QRectF(1-length,1-RPIPE,length+RPIPE-1,length+RPIPE-1);
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
             boundary = QRectF(-RPIPE,-length,length+RPIPE,length+RPIPE);
             break;
        case PIPE2_TOP_BOTTOM:
        case PIPE2_BOTTOM_TOP:
        case PIPE2_BOTTOM_TOP_NO_ARROW:
             x      = pipeLength >> 1;
             top    = QPoint(0,-x);
             bottom = -top;
             path.moveTo(+RPIPE,-x);
             path.lineTo(+RPIPE,+x);
             path.lineTo(-RPIPE,+x);
             path.lineTo(-RPIPE,-x);
             path.lineTo(+RPIPE,-x);
             boundary = QRectF(-RPIPE,-x,2*RPIPE+2,pipeLength-1);
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
             boundary = QRectF(-RPIPE,1-RPIPE,length+RPIPE,length+RPIPE-1);
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
             boundary = QRectF(-50,1-RPIPE,100,50+RPIPE-1);
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
             boundary = QRectF(-50,50,50+RPIPE,100);
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
             boundary = QRectF(-50,-50,100,50+RPIPE-1);
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
             boundary = QRectF(-RPIPE,-50,50+RPIPE,100);
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
             boundary = QRectF(-50,-50,100,100);
             break;
    } // switch
    pipePolygon = path.toFillPolygon();
    setPolygon(pipePolygon);
} // Pipe::drawPipe()

void Pipe::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;

    pen.setColor(Qt::black);
    pen.setWidth(3);
    painter->setPen(pen);

    if ((pipeType == PIPE2_LEFT_RIGHT) || (pipeType == PIPE2_RIGHT_LEFT))
    {   // draw arrow from left to right or from right to left
        int x1 = (pipeType == PIPE2_LEFT_RIGHT) ? 20 : -20;
        painter->drawLine(bottom + QPoint(-x1,20)  , bottom + QPoint(x1>>1,20));
        painter->drawLine(bottom + QPoint(x1>>1,20), bottom + QPoint(x1>>2,15));
        painter->drawLine(bottom + QPoint(x1>>2,15), bottom + QPoint(x1>>2,25));
        painter->drawLine(bottom + QPoint(x1>>2,25), bottom + QPoint(x1>>1,20));
    } // if
    else if (pipeType == PIPE2_TOP_BOTTOM)
    {   // draw arrow from top to bottom
        painter->drawLine(right + QPoint(20,-10), right + QPoint(20,20));
        painter->drawLine(right + QPoint(20,20) , right + QPoint(25,15));
        painter->drawLine(right + QPoint(25,15) , right + QPoint(15,15));
        painter->drawLine(right + QPoint(15,15) , right + QPoint(20,20));
    } // if
    else if (pipeType == PIPE2_BOTTOM_TOP)
    {   // draw arrow from bottom to top
        painter->drawLine(right + QPoint(20,+20), right + QPoint(20,-10));
        painter->drawLine(right + QPoint(20,-10), right + QPoint(25,-5));
        painter->drawLine(right + QPoint(25,-5) , right + QPoint(15,-5));
        painter->drawLine(right + QPoint(15,-5) , right + QPoint(20,-10));
    } // if
    QGraphicsPolygonItem::paint(painter,option,widget);
} // Pipe::paint()

//------------------------------------------------------------------------------------------
// Text-display object for STD with sub-text
//------------------------------------------------------------------------------------------
Display::Display(QPointF point, int w, QString title, QColor tc, QColor stc)
    : QGraphicsSimpleTextItem()
{
    QFont font;

    width  = w;
    height = 45;
    font.setPointSize(18);
    setFont(font);
    setTitleText(title);
    setTextColor(tc);
    setSubTextColor(stc);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
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
    painter->setPen(subTextColor);
    painter->drawText(5,41,subText); // Draw sub-text
    painter->setPen(Qt::black);
    painter->drawText(0,-5,titleText); // Draw title
    QGraphicsSimpleTextItem::paint(painter,option,widget);
} // Display::paint()

void Display::setTitleText(QString string)
{
    titleText = string;
} // Display::setTitleText()

void Display::setSubText(QString string)
{
    subText = string;
} // Display::setSubText()

void Display::setTextColor(QColor color)
{
    setBrush(QBrush(color));
    setPen(QPen(color));
} // setTextColor()

void Display::setSubTextColor(QColor color)
{
    subTextColor = color;
} // setSubTextColor()

//------------------------------------------------------------------------------------------
// Meter object for Flow-meter and Temp-meter
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
    setError(false);   // No error => green colour
    meterType = type;
    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        MA *p = new MA(METER_FLOW_MA_N,0.0);
        pma   = p; // save pointer
        setText("F");
        setFlowParameters(1000,false,0.0); // set defaults to 1 sec, no temp. correction
        meterValueOld = 0.0;
        setFlowValue(meterValueOld,TEMP_DEFAULT); // init. values for flowrate measurement
        initFlowRateDetector(10); // init. flowrate-low detector
    } // if
    else
    {
        setText("T");
        setTempValue(TEMP_DEFAULT); // default temperature
    } // else
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    if ((meterType == METER_HFLOW) || (meterType == METER_HTEMP))
         boundary = QRectF(-20,-15,100,100); // horizontal lay-out
    else boundary = QRectF(-35,-10,160, 60); // vertical   lay-out
    left     = QPointF(-13,+24);
    right    = QPointF(+34,+24);
    top      = QPointF(+11,0);
    bottom   = QPointF(+11,48);
} // Meter::Meter()

void Meter::setTempValue(qreal value)
{
    if (value > SENSOR_VAL_LIM_OK)
    {   // valid
        meterValue = value;
        meterError = false;
    } // if
    else
    {   // faulty or not connected
        meterValue = TEMP_DEFAULT;
        meterError = true;
    } // else
} // Meter::setValue()

void Meter::setFlowParameters(uint16_t msec, bool temp_corr, qreal flow_err)
{
    Ts                 = msec;
    if (Ts < 100.0) Ts = 100.0; // min. of 100 msec.
    tempCorrection     = temp_corr;
    flowErr            = flow_err;
} // Meter::setFlowParameters()

void Meter::setFlowValue(qreal value,qreal temp)
{
    meterValue  = value;
    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        // Apply Calibration error correction
        meterValue *= 1.0 + 0.01 * flowErr;
        // Calculate Flow-rate in L per minute: Ts [msec.]
        if (meterValue > meterValueOld)
             flowRateRaw = (60000.0 / Ts) * (meterValue - meterValueOld);
        else flowRateRaw = 0.0;
        meterValueOld = meterValue;
        flowRate = pma->moving_average(flowRateRaw);
        // Apply Temperature-correction
        if (tempCorrection)
            meterValue /= (1.0 + 0.00021 * (temp - 20.0));
    } // if
    update();
} // Meter::setValue()

qreal Meter::getFlowValue(void)
{
    return meterValue;
} // Meter::getFlowValue()

qreal Meter::getFlowRate(uint8_t fil)
{
    if (fil == FLOWRATE_FIL)
         return flowRate;    // filtered flowrate
    else return flowRateRaw; // unfiltered flowrate
} // Meter::getFlowRate()

void Meter::setName(QString name)
{
    meterName = name;
} // Meter::setName()

void Meter::setError(bool err)
{
    meterError = err;
} // Meter::setError()

QPointF Meter::getCoordinates(uint8_t side)
{
    QPointF point = pos();

    switch (side)
    {
        case COORD_TOP   : point += top;    break; /* top coordinate of pipe */
        case COORD_RIGHT : point += right;  break; /* right coordinate of pipe */
        case COORD_LEFT  : point += left;   break; /* left coordinate of pipe */
        case COORD_BOTTOM: point += bottom; break; /* bottom coordinate of pipe */
        default:           point  = QPointF(0,0); break;
    } // which
    return point;
} // Meter::getCoordinates()

void Meter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainterPath path;
    QFont        font;
    QString      text1,text2 = "";

    path.addEllipse(QRect(-14,-1,50,50));
    if (meterError)
         painter->fillPath(path,Qt::red);
    else if ((meterType == METER_HTEMP) || (meterType == METER_VTEMP))
         painter->fillPath(path,Qt::green);
    else
    {    // flowmeter
         if (flowRate < 0.1)
              painter->fillPath(path,QColor(255,165,0)); // dark orange
         else painter->fillPath(path,Qt::green);
    } // else
    painter->drawArc(QRect(-14,-1,50,50),0,5760);
    font.setPointSize(12);
    font.setBold(true);
    painter->setFont(font);

    if ((meterType == METER_HFLOW) || (meterType == METER_VFLOW))
    {
        text1 = QString("%1 L").arg(meterValue,1,'f',1);
        text2 = QString("%1 L/min.").arg(flowRate,1,'f',1);
    } // if
    else
    {
        text1 = QString("%1 °C").arg(meterValue,1,'f',2);
    } // else
    if ((meterType == METER_HFLOW) || (meterType == METER_HTEMP))
    {    // horizontal lay-out
         painter->drawText(-14,63,text1);
         if (flowRate > 0.01) painter->drawText(-14,78,text2);
    } // if
    else
    {   // vertical lay-out
        painter->drawText(+40,25,text1);
        if (flowRate > 0.01) painter->drawText(+40,40,text2);
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
            if (!meterError && (++frl_tmr > 30))
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
            {
                frl_tmr = 0; // reset timer
                frl_std = 4; // check if active for > 10 seconds
            } // if
            break;
        case 4:  // Now check if flow-rate is decreased for at least 10 seconds
            if (flowRate >= frl_det_lim) // flow-rate >= percentage of average flow-rate?
            {
                frl_tmr = 0; // reset timer
                frl_std = 3; // check again
            } // if
            else if (++frl_tmr > 10)
            {
                frl_tmr = 10;
                retv    = true;
            } // else if
            break;
        default: frl_std = 0;
                 frl_tmr = 0;
                 break;
        } // switch
    return retv;
} // Meter::isFlowRateLow()

//------------------------------------------------------------------------------------------
// Base object for Valves and Pumps
//------------------------------------------------------------------------------------------
Actuator::Actuator(QPointF point, QString name)
    : QGraphicsPolygonItem()
{
    setPos(point);
    setColor(Qt::red);
    setName(name);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemIsSelectable,true);
    //setFlag(QGraphicsItem::ItemIsMovable,true);
    left = top = right = bottom = QPoint(0,0); // init. all coordinates
} // Actuator()

// This function returns the parent coordinate of top, right, left or bottom of pipe
QPointF Actuator::getCoordinates(uint8_t side)
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
} // Actuator::getCoordinates()

void Actuator::setColor(QColor color)
{
    actuatorColor = color;
    setBrush(actuatorColor);
} // Actuator::setRadius()

void Actuator::setName(QString name)
{
    actuatorName = name;
} // Actuator::setName()

void Actuator::setStatus(uint8_t status)
{
    actuatorStatus = status;
    if ((actuatorStatus == MANUAL_OFF) || (actuatorStatus == AUTO_OFF))
         setColor(Qt::red);
    else setColor(Qt::green);
    update();
} // Actuator::setStatus()

void Actuator::setNextStatus(void)
{
    if ((actuatorStatus == AUTO_OFF) || (actuatorStatus == MANUAL_OFF))
    {
        actuatorStatus = MANUAL_ON;
    } // if
    else if ((actuatorStatus == AUTO_ON) || (actuatorStatus == MANUAL_ON))
         actuatorStatus = MANUAL_OFF;
    setStatus(actuatorStatus);
} // Actuator::setNextStatus()

bool Actuator::inManualMode(void)
{
    if ((actuatorStatus == MANUAL_OFF) || (actuatorStatus == MANUAL_ON))
         return true;
    else return false;
} // Actuator::inManualMode()

uint8_t Actuator::getStatus(void)
{
    return actuatorStatus;
} // Actuator::getStatus()

void Actuator::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *setAutoAction      = menu.addAction("Auto");
    QAction *setManualOffAction = menu.addAction("OFF (M)");
    QAction *setManualOnAction  = menu.addAction("ON (M)");
    setAutoAction->setCheckable(true);
    setManualOffAction->setCheckable(true);
    setManualOnAction->setCheckable(true);
    if      (!inManualMode())              setAutoAction->setChecked(true);
    else if (actuatorStatus == MANUAL_OFF) setManualOffAction->setChecked(true);
    else if (actuatorStatus == MANUAL_ON)  setManualOnAction->setChecked(true);
    QAction *selectedAction = menu.exec(event->screenPos());
    if      (selectedAction == setAutoAction)      setStatus(AUTO_OFF);
    else if (selectedAction == setManualOffAction) setStatus(MANUAL_OFF);
    else if (selectedAction == setManualOnAction)  setStatus(MANUAL_ON);
} // Actuator::contextMenuEvent()

/*------------------------------------------------------------------
  Purpose  : This function sets actionBits to the proper setting (on,off)
             indicated by whichBit in case Manual mode is selected.
             In Auto mode, the object status is set to the value in
             actionBits.
  Variables:
 actionBits: every actuator has its own defined bitvalue here
   whichBit: a bitdefine for the specific actuator (e.g. V1b = 0x0001)
  Returns  : -
  ------------------------------------------------------------------*/
void Actuator::setActuator(uint16_t& actionBits, uint16_t whichBit)
{
    if (!inManualMode())
    { // not in Manual Override mode
        if (actionBits & whichBit) setStatus(AUTO_ON);
        else                       setStatus(AUTO_OFF);
    } // if
    else
    {  // manual Override mode
       if (getStatus() == MANUAL_ON)
            actionBits |=  whichBit;
       else actionBits &= ~whichBit;
    } // else
} // Actuator::setActuator()

//------------------------------------------------------------------------------------------
// Valve object, derived from Actuator
//------------------------------------------------------------------------------------------
Valve::Valve(QPointF point, bool orientation, QString name)
    : Actuator(point,name)
{
    setOrientation(orientation); // Draw the valve
    actuatorStatus = AUTO_OFF;
} // Valve::Valve()

void Valve::setOrientation(bool orientation)
{
    QPainterPath path;
    float x1 = qSqrt(0.5)*RVALVE;

    actuatorOrientation = orientation;
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
        boundary = QRectF(-60-(VALVE_SIZE>>1),-(VALVE_SIZE>>1),60+VALVE_SIZE,VALVE_SIZE); // extra space left for title and status
    } // else
    actuatorPolygon = path.toFillPolygon();
    setPolygon(actuatorPolygon);
} // Valve::SetValveOrientation()

void Valve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QFont font;
    font.setStyleHint(QFont::Times, QFont::PreferAntialias);
    font.setBold(true);
    font.setPointSize(20);
    painter->setFont(font);
    painter->save();
    if (actuatorOrientation == HORIZONTAL)
    {
        painter->drawText(-0.35*VALVE_SIZE,-1.5*RVALVE,actuatorName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-0.5*VALVE_SIZE,+0.8*VALVE_SIZE,statustext[actuatorStatus]);
    } // if
    else
    {   // VERTICAL
        painter->drawText(-1.1*VALVE_SIZE,5-1.2*RVALVE,actuatorName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-1.4*VALVE_SIZE,5+0.7*RVALVE,statustext[actuatorStatus]);
    } // else
    painter->restore();
    QGraphicsPolygonItem::paint(painter,option,widget);
} // Valve::paint()

//------------------------------------------------------------------------------------------
// Pump object, derived from Actuator
//------------------------------------------------------------------------------------------
Pump::Pump(QPointF point, bool orientation, QString name)
    : Actuator(point,name)
{
    setPumpOrientation(orientation); // Draw the valve
    actuatorStatus = AUTO_OFF;
} // Pump()

void Pump::setPumpOrientation(bool orientation)
{
    QPainterPath path;
    double x1  = qSqrt(RPUMP*RPUMP - RPIPE*RPIPE); // x-coord of pipe-in with PUMP at left side
    double x1a = 180 - qAtan2(RPIPE,x1) * 180 / M_PI;
    double x2  = qSqrt(10*RPUMP - 25); // x-coord at y=25: pipe-out with PUMP at right side
    double x2a = qAtan2(RPUMP-5,x2) * 180 / M_PI;
    double y3  = RPUMP-5-2*RPIPE;
    double x3  = qSqrt(RPUMP*RPUMP-(5-RPUMP+2*RPIPE)*(5-RPUMP+2*RPIPE));
    double x3a = qAtan2(y3,x3) * 180 / M_PI;

    actuatorOrientation = orientation;
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
        boundary = QRectF(-60,-(RPUMP<<1),120,RPUMP<<2); // extra space on top and bottom for title and status
    } // if
    else
    {   // TODO Pump output is on the left
    } // else
    actuatorPolygon = path.toFillPolygon();
    setPolygon(actuatorPolygon);
} // Pump::SetPumpOrientation()

void Pump::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QFont font;
    QPen  pen;

    font.setStyleHint(QFont::Times, QFont::PreferAntialias);
    font.setBold(true);
    font.setPointSize(20);
    pen.setWidth(1);

    painter->setFont(font);
    painter->save();
    if (actuatorOrientation == OUT_RIGHT)
    {
        painter->drawText(-20,-40,actuatorName);
        font.setPointSize(14);
        painter->setFont(font);
        painter->drawText(-35,RPUMP+25,statustext[actuatorStatus]);
        font.setPointSize(10);
        painter->setFont(font);
        painter->drawText(-50,22,"in");
        painter->drawText(38,+3,"out");
    } // if
    painter->restore();
    QGraphicsPolygonItem::paint(painter,option,widget);
    painter->setBrush(COLOR_BACKGROUND);
    painter->drawChord(-10,-10,20,20,0,5760); // hole in the middle
} // Pump::paint()


