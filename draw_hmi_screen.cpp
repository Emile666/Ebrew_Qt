/**************************************************************************************
** Filename    : draw_hmi_screen.cpp
** Author      : Emile
** Purpose     : This file contains a single procedure that
**               draws all graphical objects to the screen.
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
#include <QGraphicsScene>
#include "hmi_objects.h"
#include "MainEbrew.h"

/*-----------------------------------------------------------------------------
  Purpose    : Draw the entire Human-Machine-Interface (HMI) screen with the
               tanks, valves, pumps, lines and meters.
  Variables  : scene: pointer to QGraphicsScene object in main()
               p    : pointer to MainEbrew object in main()
  Returns    : -
  ---------------------------------------------------------------------------*/
void draw_hmi_screen(QGraphicsScene *scene, MainEbrew *p)
{
    QPointF point;
    int     len;
    QFont   font;

    //----------------------------------------------------------------------
    // Draw HLT tank with Pump 2
    // Objects: hlt, pump2, elbowP20, elbowP21,  pipeH1, elbowP22, elbowP23
    //----------------------------------------------------------------------
    Tank *hlt = new Tank(-700,0,250,300,TANK_EXIT_BOTTOM|TANK_HEAT_EXCHANGER,"HLT 200L");
    scene->addItem(hlt); // Hot-Liquid Tun (HLT)
    p->hlt = hlt;        // save hlt reference to MainEbrew
    point = hlt->getCoordinates(COORD_LEFT_PIPE1) + QPoint(-60,20); // get coordinates of top-left pipe of heat-exchanger
    hlt->setToolTip("<b>Hot Liquid Tun (HLT)</b>: contains fresh water for the brew system and has a built-in temperature sensor and a heat-exchanger.<br>The following information is shown:<br>1) The actual temperature and the setpoint temperature.<br>2) Actual volume showing the remaining water in the HLT.<br>3) Actual power applied to the HLT, this is the PID controller output as a percentage from 0 to 100 %.");

    Pump *pump2 = new Pump(point,OUT_RIGHT,"P2");
    pump2->setStatus(AUTO_OFF);
    pump2->setToolTip("<b>HLT pump</b>: controls water circulation inside HLT and pumps water through internal HLT heat-exchanger. Red=Off, Green=Running. Press \'<b>Q</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(pump2);
    p->P2 = pump2;            // add pump2 reference to MainEbrew
    point = pump2->getCoordinates(COORD_LEFT) + QPoint(-50,0);
    Pipe *elbowP20 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_IN0);
    p->elbowP20 = elbowP20;   // add reference to MainEbrew
    scene->addItem(elbowP20); // elbow pipe at left of pump 2
    point = elbowP20->getCoordinates(COORD_BOTTOM) + QPoint(0,50);
    Pipe *elbowP21 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    p->elbowP21 = elbowP21;   // add reference to MainEbrew
    scene->addItem(elbowP21); // elbow pipe bottom-left of pump 2

    point = hlt->getCoordinates(COORD_LEFT_PIPE2) + QPoint(-6,0); // get coordinates of bottom-left pipe of heat-exchanger
    Pipe *elbowP23 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_IN0);
    p->elbowP23 = elbowP23;     // add reference to MainEbrew
    scene->addItem(elbowP23);
    point = elbowP23->getCoordinates(COORD_BOTTOM) + QPoint(0,25);
    point.setY(elbowP21->y());  // same y coordinate as elbow P21
    Pipe *elbowP22 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_IN0);
    p->elbowP22 = elbowP22;     // add reference to MainEbrew
    scene->addItem(elbowP22);   // elbow-pipe bottom-right of pump 2
    point = 0.5*(elbowP21->getCoordinates(COORD_RIGHT) + elbowP22->getCoordinates(COORD_LEFT)); // x-midpoint between pipeH1 and pipe 3
    len   = (elbowP22->getCoordinates(COORD_LEFT) - elbowP21->getCoordinates(COORD_RIGHT)).x();
    Pipe *pipeH1 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    p->pipeH1 = pipeH1;         // add reference to MainEbrew
    scene->addItem(pipeH1);     // connecting pipe below pump 2

    //----------------------------------------------------------
    // Draw HLT output pipe with V2
    // Objects: valve2, elbow2, flow1, Tpipe1
    //----------------------------------------------------------
    point = hlt->getCoordinates(COORD_BOTTOM_PIPE1) + QPoint(0,30); // get coordinates of bottom-left pipe
    Valve *valve2 = new Valve(point,VERTICAL,"V2");
    valve2->setToolTip("<b>Solenoid ball valve V2</b>, HLT-output, red=closed, green=open. Press \'<b>2</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(valve2);
    p->V2 = valve2;          // Add valve2 reference to MainEbrew
    point = valve2->getCoordinates(COORD_BOTTOM) + QPoint(0,50);
    Pipe *elbow2 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    p->elbow2 = elbow2;      // add reference to MainEbrew
    scene->addItem(elbow2);  // elbow pipe below valve V2
    point = elbow2->getCoordinates(COORD_RIGHT) + QPoint(12,-25);
    Meter *flow1 = new Meter(point,METER_HFLOW,"flow1");
    flow1->setFlowParameters(TS_FLOWS_MSEC,p->RegEbrew->value("FLOW_TEMP_CORR").toInt()==1,p->RegEbrew->value("FLOW1_ERR").toDouble());
    flow1->setToolTip("<b>Flowsensor 1</b>: measures water volume (L) coming from HLT: Red=Error, Orange=No flow, Green=Flow detected.<br>Calibrate sensor with Options->Measurements Settings->Flows Dialog screen");
    p->F1 = flow1;           // Add flow1 reference to MainEbrew
    point = flow1->getCoordinates(COORD_RIGHT) + QPoint(50,1);
    Pipe *Tpipe1 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_IN0); // T-pipe right of flowmeter 1
    p->Tpipe1 = Tpipe1;      // add reference to MainEbrew
    scene->addItem(Tpipe1);  // First, draw both pipes, then add flowmeter!
    scene->addItem(flow1);   // Flowmeter between HLT and Pump 1 input

    //----------------------------------------------------------
    // Draw MLT tank with output pipe and valve V1
    // Objects: mlt, valve1, Tpipe2, pipeH2
    //----------------------------------------------------------
    Tank *mlt = new Tank(-270,0,250,300,TANK_MANIFOLD_BOTTOM|TANK_MANIFOLD_TOP,"MLT 110L");
    mlt->setToolTip("<b>Mash Lauter Tun (MLT)</b>: contains the grains and the wort and has a built-in temperature sensor. The MLT is heated indirectly, which is what it makes a HERMS system.<br>The following information is shown:<br>1) The actual temperature and the setpoint temperature.<br>2) Actual volume showing the wort volume in the MLT.<br>");
    scene->addItem(mlt);     // Mash-Lauter Tun (MLT)
    p->mlt = mlt;            // add MLT reference to MainEbrew
    point = mlt->getCoordinates(COORD_BOTTOM_PIPE1) + QPoint(0,30); // get coordinates of bottom-left pipe
    Valve *valve1 = new Valve(point,VERTICAL,"V1");
    valve1->setToolTip("<b>Solenoid ball valve V1</b>, MLT output, red=closed, green=open. Press \'<b>1</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(valve1);  // valve V1
    p->V1 = valve1;          // add valve1 reference to MainEbrew
    point += QPoint(0,80);
    Pipe *Tpipe2 = new Pipe(point,PIPE3_NO_BOTTOM,50,COLOR_IN0);
    p->Tpipe2 = Tpipe2;      // add reference to MainEbrew
    scene->addItem(Tpipe2);  // T-pipe below valve V1
    point = 0.5*(Tpipe2->getCoordinates(COORD_LEFT) + Tpipe1->getCoordinates(COORD_RIGHT)); // x-midpoint between Tpipe1 and pipe 5
    len   = (Tpipe2->getCoordinates(COORD_LEFT) - Tpipe1->getCoordinates(COORD_RIGHT)).x(); // x-length
    Pipe *pipeH2 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    p->pipeH2 = pipeH2;         // add reference to MainEbrew
    scene->addItem(pipeH2);   // horizontal connecting pipe between HLT and MLT

    //----------------------------------------------------------
    // Draw Boil-kettle with output pipe and valve V3
    // Objects: boil, valve3, elbow3, pipeH3
    //----------------------------------------------------------
    Tank *boil = new Tank(60,0,275,300,TANK_MANIFOLD_BOTTOM|TANK_RETURN_BOTTOM,"BOIL 140L");
    scene->addItem(boil);
    boil->setToolTip("<b>Boil-kettle (BOIL)</b>: contains the boiling wort and has a built-in temperature sensor.<br>The following information is shown:<br>1) The actual temperature and the setpoint temperature.<br>2) Actual volume showing the wort volume in the Boil-kettle.<br>3) Actual power applied to the Boil-kettle, this is the PID controller output as a percentage from 0 to 100 %.");
    p->boil = boil;          // add boil reference to MainEbrew
    point = boil->getCoordinates(COORD_BOTTOM_PIPE1) + QPoint(0,30); // get coordinates of bottom-left pipe
    Valve *valve3 = new Valve(point,VERTICAL,"V3");
    valve3->setToolTip("<b>Solenoid ball valve V3</b>, Boil-kettle output, red=closed, green=open. Press \'<b>3</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(valve3);  // Valve V3 at input-side of pump
    p->V3 = valve3;          // add valve3 reference to MainEbrew
    point += QPoint(0,80);
    Pipe *elbow3 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_IN0);
    p->elbow3 = elbow3;      // add reference to MainEbrew
    scene->addItem(elbow3);  // elbow pipe below valve V3, needs to be connected to Tpipe2 (T-pipe below valve V1)
    point = 0.5*(elbow3->getCoordinates(COORD_LEFT) + Tpipe2->getCoordinates(COORD_RIGHT)); // x-midpoint between pipe 5 and pipe 6
    len   = (elbow3->getCoordinates(COORD_LEFT) - Tpipe2->getCoordinates(COORD_RIGHT)).x();
    Pipe *pipeH3 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    p->pipeH3 = pipeH3;      // add reference to MainEbrew
    scene->addItem(pipeH3);  // horizontal connecting pipe between MLT and Boil-kettle

    //----------------------------------------------------------
    // Draw Boil-kettle input pipe with valve V7
    // Objects: valve7, pipeV5, elbow4, pipeH8, flow2, Tpipe4
    //----------------------------------------------------------
    point = boil->getCoordinates(COORD_BOTTOM_PIPE2) + QPoint(0,30); // get coordinates of bottom-right pipe
    Valve *valve7 = new Valve(point,VERTICAL,"V7");
    valve7->setToolTip("<b>Solenoid ball valve V7</b>, Boil-kettle input, red=closed, green=open. Press \'<b>7</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(valve7);  // valve V7
    p->V7 = valve7;          // add valve7 reference to MainEbrew
    point = valve7->getCoordinates(COORD_BOTTOM) + QPoint(0,60);
    Pipe *pipeV5 = new Pipe(point,PIPE2_BOTTOM_TOP,120,COLOR_OUT0);
    scene->addItem(pipeV5);  // vertical pipe below valve V7
    p->pipeV5 = pipeV5;      // add reference to MainEbrew
    point = pipeV5->getCoordinates(COORD_BOTTOM);
    Pipe *elbow4 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    p->elbow4 = elbow4;      // add reference to MainEbrew
    scene->addItem(elbow4);  // elbow below valve V7
    point = elbow4->getCoordinates(COORD_LEFT) + QPoint(-25,0);
    Pipe *pipeH8 = new Pipe(point,PIPE2_LEFT_RIGHT,50,COLOR_OUT0);
    p->pipeH8 = pipeH8;      // add reference to MainEbrew
    scene->addItem(pipeH8);  // horizontal pipe between V6 and V7
    point = pipeH8->getCoordinates(COORD_LEFT) + QPoint(-34,-25);
    Meter *flow2 = new Meter(point,METER_HFLOW,"flow2");
    flow2->setFlowParameters(TS_FLOWS_MSEC,p->RegEbrew->value("FLOW_TEMP_CORR").toInt()==1,p->RegEbrew->value("FLOW2_ERR").toDouble());
    flow2->setToolTip("<b>Flowsensor 2</b>: measures wort volume (L) flowing into Boil-kettle: Red=Error, Orange=No flow, Green=Flow detected.<br>Calibrate sensor with Options->Measurements Settings->Flows Dialog screen");
    p->F2 = flow2;           // Add flow2 reference to MainEbrew
    point = flow2->getCoordinates(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe4 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);
    p->Tpipe4 = Tpipe4;      // add reference to MainEbrew
    scene->addItem(Tpipe4);  // T-pipe at top of V6
    scene->addItem(flow2);   // First, add both pipes, then add flowmeter 2

    //----------------------------------------------------------
    // Draw vertical pipe down to V6 and CFC-output
    // Objects: valve6, pipeV4, flow3, elbow5, temp3, pipeH7
    //----------------------------------------------------------
    point = Tpipe4->getCoordinates(COORD_BOTTOM) + QPoint(0,30);
    Valve *valve6 = new Valve(point,VERTICAL,"V6");
    scene->addItem(valve6);  // valve V6
    valve6->setToolTip("<b>Solenoid ball valve V6</b>, CFC-output, red=closed, green=open. Press \'<b>6</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    p->V6 = valve6;          // add valve6 reference to MainEbrew
    point = valve6->getCoordinates(COORD_BOTTOM) + QPoint(0,20);

    Pipe *pipeV4 = new Pipe(point,PIPE2_TOP_BOTTOM,40,COLOR_OUT0);
    p->pipeV4 = pipeV4;      // add reference to MainEbrew
    scene->addItem(pipeV4);  // vertical pipe between V6 and CFC
    point = pipeV4->getCoordinates(COORD_BOTTOM) + QPoint(-11,-1);

    Meter *flow3 = new Meter(point,METER_VFLOW,"flow3");
    flow3->setFlowParameters(TS_FLOWS_MSEC,p->RegEbrew->value("FLOW_TEMP_CORR").toInt()==1,p->RegEbrew->value("FLOW3_ERR").toDouble());
    flow3->setToolTip("<b>Flowsensor 3</b>: measures wort volume (L) flowing into Fermentation Bin: Red=Error, Orange=No flow, Green=Flow detected.<br>Calibrate sensor with Options->Measurements Settings->Flows Dialog screen");
    point = flow3->getCoordinates(COORD_BOTTOM) + QPoint(0,48);
    p->F3 = flow3;           // Add flow3 reference to MainEbrew

    Pipe *elbow5 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    p->elbow5 = elbow5;      // add reference to MainEbrew
    scene->addItem(elbow5);  // Exit pipe from flow3 to CFC
    scene->addItem(flow3);

    point = elbow5->getCoordinates(COORD_RIGHT) + QPoint(13,-25);
    Meter *temp3 = new Meter(point,METER_HTEMP,"Tcfc");
    temp3->setToolTip("<b>Temperature sensor 3</b>: measures temperature (°C) at output of Counterflow Chiller (CFC). Use it to control wort temperature flowing into fermentation bin. Red=Error reading sensor, Green=No error. Calibrate sensor with Options->Measurements Settings->Temperatures Dialog screen");
    scene->addItem(temp3);
    p->T3 = temp3;           // Add temp3 reference to MainEbrew

    point = temp3->getCoordinates(COORD_RIGHT) + QPoint(26,0);
    Pipe *pipeH7 = new Pipe(point,PIPE2_LEFT_RIGHT,50,COLOR_OUT0);
    p->pipeH7 = pipeH7;      // add reference to MainEbrew
    scene->addItem(pipeH7);  // horizontal pipe between CFC and output

    point += QPoint(-140,40);
    font.setPointSize(12);
    font.setBold(true);
    QGraphicsTextItem *text = scene->addText("To Fermenter",font);
    text->setPos(point);     // insert text

    //----------------------------------------------------------
    // Draw left from T-pipe (Tpipe4) and left from valve V6
    // Objects: pipeH6, Tpipe3
    //----------------------------------------------------------
    point = Tpipe4->getCoordinates(COORD_LEFT) + QPoint(-50,0);
    Pipe *pipeH6 = new Pipe(point,PIPE2_LEFT_RIGHT,100,COLOR_OUT0);
    p->pipeH6 = pipeH6;      // add reference to MainEbrew
    scene->addItem(pipeH6);  // horizontal pipe at left of Valve V6
    point = pipeH6->getCoordinates(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe3 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);
    p->Tpipe3 = Tpipe3;      // add reference to MainEbrew
    scene->addItem(Tpipe3);  // First T-pipe at output of pump 1
    point = Tpipe3->getCoordinates(COORD_BOTTOM) + QPoint(0,50);

    //----------------------------------------------------------
    // Draw from T-pipe (Tpipe3) down to Pump 1
    // Objects: elbow6, pump1
    //----------------------------------------------------------
    Pipe *elbow6 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    p->elbow6 = elbow6;      // add reference to MainEbrew
    scene->addItem(elbow6);  // elbow pipe right of pump 1
    point = elbow6->getCoordinates(COORD_LEFT) + QPoint(-60,20);
    Pump *pump1 = new Pump(point,OUT_RIGHT,"P1");
    pump1->setStatus(AUTO_OFF);
    pump1->setToolTip("<b>Main pump</b> for the HERMS brewing system. Red=Off, Green=Running. Press \'<b>P</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(pump1);   // Pump 1
    p->P1 = pump1;           // Add pump1 reference to MainEbrew

    //----------------------------------------------------------
    // Draw left-input connection of Pump 1 (main-pump)
    // Objects: elbow7, pipeV1, pipeH4
    //----------------------------------------------------------
    point.setX(Tpipe1->x()); // Elbow pipe elbow7 should have the x-coordinate of the T-pipe (Tpipe1) right of flowmeter 1
    point.setY(pump1->y());  // ... and the y-coordinate of pump 1
    Pipe *elbow7 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    p->elbow7 = elbow7;      // add reference to MainEbrew
    scene->addItem(elbow7);  // elbow pipe at  left (input) of pump 1

    // Now fit vertical pipe pipeV1 between Tpipe1 and elbow7
    point = 0.5*(elbow7->getCoordinates(COORD_TOP) + Tpipe1->getCoordinates(COORD_BOTTOM)); // y-midpoint between both pipes
    len   = (elbow7->getCoordinates(COORD_TOP) - Tpipe1->getCoordinates(COORD_BOTTOM)).y();
    Pipe *pipeV1 = new Pipe(point,PIPE2_TOP_BOTTOM,len,COLOR_IN0);
    p->pipeV1 = pipeV1;      // add reference to MainEbrew
    scene->addItem(pipeV1);  // vertical pipe connecting pump input to main system

    // Now fit horizontal pipe pipeH4 between elbow-pipe elbow7 and pump1
    point = 0.5*(pump1->getCoordinates(COORD_LEFT) + elbow7->getCoordinates(COORD_RIGHT)); // x-midpoint between pipe and pump
    len   = (pump1->getCoordinates(COORD_LEFT) - elbow7->getCoordinates(COORD_RIGHT)).x();
    Pipe *pipeH4 = new Pipe(point,PIPE2_LEFT_RIGHT,len,COLOR_IN0);
    p->pipeH4 = pipeH4;
    scene->addItem(pipeH4);  // horizontal pipe left of pump1

    //----------------------------------------------------------
    // Connect HLT heat-exchanger output to MLT return-manifold
    // Objects: elbow8, flow4, elbow9
    //----------------------------------------------------------
    point = hlt->getCoordinates(COORD_RIGHT_PIPE1) + QPoint(50,0); // get coordinates of top-right pipe of heat-exchanger
    Pipe *elbow8 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    p->elbow8 = elbow8;      // add reference to MainEbrew
    scene->addItem(elbow8);  // lower elbow pipe right of HLT heat-exchanger output
    // Now calculate flowmeter 4 position, but don't add it yet
    point = elbow8->getCoordinates(COORD_TOP) + QPoint(-11,-47);
    Meter *flow4 = new Meter(point,METER_VFLOW,"flow4");
    flow4->setFlowParameters(TS_FLOWS_MSEC,p->RegEbrew->value("FLOW_TEMP_CORR").toInt()==1,p->RegEbrew->value("FLOW4_ERR").toDouble());
    flow4->setToolTip("<b>Flowsensor 4</b>: measures water volume (L) flowing into top of MLT: Red=Error, Orange=No flow, Green=Flow detected.<br>Calibrate sensor with Options->Measurements Settings->Flows Dialog screen");
    // Calculate size for top-left elbow, so that a separate vertical pipe is not needed
    point.setX(elbow8->x()); // set x-coordinate to lower elbow pipe
    point.setY(mlt->getCoordinates(COORD_LEFT_TOP_PIPE).y()); // get y-coordinate of top-left pipe for return-manifold in top
    int lenx = (mlt->getCoordinates(COORD_LEFT_TOP_PIPE) - flow4->getCoordinates(COORD_TOP)).x(); // x-diff between flowmeter and top elbow pipe
    int leny = (flow4->getCoordinates(COORD_TOP) - mlt->getCoordinates(COORD_LEFT_TOP_PIPE)).y(); // y-diff between flowmeter and top elbow pipe
    len = qMax(lenx,leny);   // use the larger of the two
    Pipe *elbow9 = new Pipe(point,PIPE2_BOTTOM_RIGHT,len,COLOR_OUT0);
    p->elbow9 = elbow9;      // add reference to MainEbrew
    scene->addItem(elbow9);  // top elbow pipe, left of MLT return-manifold input
    scene->addItem(flow4);   // add flow4 after pipes have been added
    p->F4 = flow4;           // add flow4 reference to MainEbrew

    //----------------------------------------------------------
    // From T-pipe 3 going left to HLT heat-exchanger input
    // Objects: pipeH5, elbow10, elbow11
    //----------------------------------------------------------
    point = Tpipe3->getCoordinates(COORD_LEFT) + QPoint(-75,0);
    Pipe *pipeH5 = new Pipe(point,PIPE2_RIGHT_LEFT,150,COLOR_OUT0);
    p->pipeH5 = pipeH5;        // add reference to MainEbrew
    scene->addItem(pipeH5);    // horizontal pipe left of Tpipe3 at Pump 1 output
    point = pipeH5->getCoordinates(COORD_LEFT) + QPoint(-25,0);
    Pipe *elbow10 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    p->elbow10 = elbow10;      // add reference to MainEbrew
    scene->addItem(elbow10);   // elbow pipe at pump 1 output, left of Tpipe3
    point.setY(elbowP23->y()); // set Y-coordinate to same value as elbow pipe 2
    Pipe *elbow11 = new Pipe(point,PIPE2_LEFT_BOTTOM,50,COLOR_OUT0);
    p->elbow11 = elbow11;      // add reference to MainEbrew
    scene->addItem(elbow11);   // elbow pipe connecting to lower pipe of HLT heat-exchanger

    //-------------------------------------------------------------------------------------------------
    // Connect Valve V4 to pump output (elbow10) and to HLT heat-exchanger input (elbow11)
    // V4 should have same y-coordinate as other valves, but with x-coordinate from elbow10/elbow11
    // Objects: pipeV3, valve4, pipeV2, pipeH9
    //-------------------------------------------------------------------------------------------------
    point.setY(valve1->y()); // get y-coordinate from one of the valves
    Valve *valve4 = new Valve(point,VERTICAL,"V4");
    valve4->setToolTip("<b>Solenoid ball valve V4</b>, MLT-return valve, red=closed, green=open. Press \'<b>4</b>\' for manual control, \'<b>A</b>\' for automated control. Click <b>right mouse button</b> for popup menu");
    scene->addItem(valve4);  // valve V4
    p->V4 = valve4;          // add valve4 reference to MainEbrew

    // Connect elbow pipe 10 to bottom of valve V4
    point = 0.5*(elbow10->getCoordinates(COORD_TOP) + valve4->getCoordinates(COORD_BOTTOM)); // y-midpoint between pipe 21 and valve 4
    len   = (elbow10->getCoordinates(COORD_TOP) - valve4->getCoordinates(COORD_BOTTOM)).y(); // y-length
    Pipe *pipeV3 = new Pipe(point,PIPE2_BOTTOM_TOP_NO_ARROW,len,COLOR_OUT0);
    p->pipeV3 = pipeV3;         // add reference to MainEbrew
    scene->addItem(pipeV3);

    // Connect top of Valve V4 to elbow pipe 11 (that connects to lower pipe of HLT heat-exchanger)
    point = 0.5*(valve4->getCoordinates(COORD_TOP) + elbow11->getCoordinates(COORD_BOTTOM)); // y-midpoint between valve 4 and pipe 22
    len   = (valve4->getCoordinates(COORD_TOP) - elbow11->getCoordinates(COORD_BOTTOM)).y(); // y-length
    Pipe *pipeV2 = new Pipe(point,PIPE2_BOTTOM_TOP,len,COLOR_OUT0);
    p->pipeV2 = pipeV2;         // add reference to MainEbrew
    scene->addItem(pipeV2);

    // Now connect lower pipe of HLT heat-exchanger to elbow 11
    point = 0.5*(elbow11->getCoordinates(COORD_LEFT) + hlt->getCoordinates(COORD_RIGHT_PIPE2)); // x-midpoint between hlt pipe-output and pipe 22
    len   = (elbow11->getCoordinates(COORD_LEFT) - hlt->getCoordinates(COORD_RIGHT_PIPE2)).x(); // x-length
    Pipe *pipeH9 = new Pipe(point,PIPE2_RIGHT_LEFT,len+2,COLOR_OUT0);
    p->pipeH9 = pipeH9;         // add reference to MainEbrew
    scene->addItem(pipeH9);

    Display *stdText = new Display(QPointF(-800,450),700);
    stdText->setToolTip("<b>Current State</b>: shows the current state of the brew-session, together with a hint of what to do next. If <b>(M)</b> is shown, it indicates that a manual action is needed");
    stdText->setText("00. Initialisation");
    stdText->setSubText("Press the HLT PID Controller button to advance to the next state");
    scene->addItem(stdText);
    p->stdText = stdText;

    PowerButton *hltPid = new PowerButton(-850,200,270,60,"HLT PID Controller");
    hltPid->setToolTip("<b>HLT Power Switch</b>: switches the HLT PID controller On or Off. Press it after power-up to start a new brew-session");
    scene->addWidget(hltPid);
    p->hltPid = hltPid; // add hltPid reference to MainEbrew

    PowerButton *boilPid = new PowerButton(-850,270,270,60,"BOIL PID Controller");
    boilPid->setToolTip("<b>Boil-kettle Power Switch</b>: switches the PID controller for the Boil-kettle On or Off. Use it if you want to manually control the Boil-kettle");
    scene->addWidget(boilPid);
    p->boilPid = boilPid; // add boilPid reference to MainEbrew

    p->setKettleNames(); // Init. titles of kettles with volumes found in Registry

} // draw_hmi_screen()

