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
#include <QGraphicsView>

#include "hmi_objects.h"

/*-----------------------------------------------------------------------------
  Purpose    : Draw the entire Human-Machine-Interface (HMI) screen with the
               tanks, valves, pumps, lines and meters.
  Variables  : -
  Returns    : -
  ---------------------------------------------------------------------------*/
void draw_hmi_screen(QGraphicsScene *scene)
{
    QPointF point;
    int     len;

    //----------------------------------
    // Draw HLT tank with Pump 2 and V2
    //----------------------------------
    Tank *hlt = new Tank(-700,0,250,300,TANK_EXIT_BOTTOM|TANK_HEAT_EXCHANGER,"HLT 200L");
    scene->addItem(hlt); // Hot-Liquid Tun (HLT)
    point = hlt->get_coordinate(COORD_LEFT_PIPE1) + QPoint(-60,20); // get coordinates of top-left pipe of heat-exchanger
    Pump *pump2 = new Pump(point,OUT_RIGHT,"P2");
    pump2->setStatus(AUTO_OFF);
    scene->addItem(pump2);
    point = pump2->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *pipe0 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe0); // elbow pipe at left of pump 2
    point = pipe0->get_coordinate(COORD_BOTTOM) + QPoint(0,50);
    Pipe *pipe1 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe1); // elbow pipe bottom-left of pump 2

    point = hlt->get_coordinate(COORD_LEFT_PIPE2) + QPoint(-6,0); // get coordinates of bottom-left pipe of heat-exchanger
    Pipe *pipe2 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe2);
    point = pipe2->get_coordinate(COORD_BOTTOM) + QPoint(0,25);
    point.setY(pipe1->y()); // same y coordinate as pipe 1
    Pipe *pipe3 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(pipe3); // elbow-pipe bottom-right of pump 2
    point = 0.5*(pipe1->get_coordinate(COORD_RIGHT) + pipe3->get_coordinate(COORD_LEFT)); // x-midpoint between pipe1 and pipe 3
    len   = (pipe3->get_coordinate(COORD_LEFT) - pipe1->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe3a = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_OUT0);
    scene->addItem(pipe3a); // connecting pipe below pump 2

    point = hlt->get_coordinate(COORD_BOTTOM_PIPE1) + QPoint(-RPIPE,30); // get coordinates of bottom-left pipe
    Valve *valve2 = new Valve(point,VERTICAL,"V2");
    scene->addItem(valve2);
    point = valve2->get_coordinate(COORD_BOTTOM) + QPoint(0,50);
    Pipe *pipe4 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    scene->addItem(pipe4); // elbow pipe below valve V2
    point = pipe4->get_coordinate(COORD_RIGHT) + QPoint(12,-25);
    Meter *flow1 = new Meter(point,METER_HFLOW,"flow1");
    flow1->setError(false);
    flow1->setValue(0.0);
    point = flow1->get_coordinate(COORD_RIGHT) + QPoint(50,1);
    Pipe *Tpipe1 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_IN0); // T-pipe right of flowmeter 1
    scene->addItem(Tpipe1); // First, draw both pipes, then add flowmeter!
    scene->addItem(flow1); // Flowmeter between HLT and Pump 1 input

    //------------------------
    // Draw MLT tank with V1
    //------------------------
    Tank *mlt = new Tank(-270,0,250,300,TANK_MANIFOLD_BOTTOM|TANK_MANIFOLD_TOP,"MLT 110L");
    scene->addItem(mlt); // Mash-Lauter Tun (MLT)
    point = mlt->get_coordinate(COORD_BOTTOM_PIPE1); // get coordinates of bottom-left pipe
    point += QPoint(-RPIPE,30);
    Valve *valve1 = new Valve(point,VERTICAL,"V1");
    scene->addItem(valve1); // valve V1
    point += QPoint(0,80);
    Pipe *Tpipe2 = new Pipe(point,PIPE3_NO_BOTTOM,50,COLOR_IN0);
    scene->addItem(Tpipe2); // T-pipe below valve V1
    point = 0.5*(Tpipe2->get_coordinate(COORD_LEFT) + Tpipe1->get_coordinate(COORD_RIGHT)); // x-midpoint between Tpipe1 and pipe 5
    len   = (Tpipe2->get_coordinate(COORD_LEFT) - Tpipe1->get_coordinate(COORD_RIGHT)).x(); // x-length
    Pipe *pipe5l = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    scene->addItem(pipe5l); // horizontal connecting pipe between HLT and MLT

    //---------------------------------------
    // Draw boil-kettle with valves V3 and V7
    //---------------------------------------
    Tank *boil = new Tank(60,0,275,300,TANK_MANIFOLD_BOTTOM|TANK_RETURN_BOTTOM,"BOIL 140L");
    scene->addItem(boil);
    point = boil->get_coordinate(COORD_BOTTOM_PIPE1) + QPoint(-RPIPE,30); // get coordinates of bottom-left pipe
    Valve *valve3 = new Valve(point,VERTICAL,"V3");
    scene->addItem(valve3); // Valve V3 at input-side of pump
    point += QPoint(0,80);
    Pipe *pipe6 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_IN0);
    scene->addItem(pipe6); // elbow pipe below valve V3, needs to be connected to pipe4 (T-pipe below valve V1)
    point = 0.5*(pipe6->get_coordinate(COORD_LEFT) + Tpipe2->get_coordinate(COORD_RIGHT)); // x-midpoint between pipe 5 and pipe 6
    len   = (pipe6->get_coordinate(COORD_LEFT) - Tpipe2->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe6l = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    scene->addItem(pipe6l); // horizontal connecting pipe between MLT and Boil-kettle

    point = boil->get_coordinate(COORD_BOTTOM_PIPE2) + QPoint(-RPIPE,30); // get coordinates of bottom-right pipe
    Valve *valve7 = new Valve(point,VERTICAL,"V7");
    scene->addItem(valve7); // valve V7
    point = valve7->get_coordinate(COORD_BOTTOM) + QPoint(0,60);
    Pipe *pipe7 = new Pipe(point,PIPE2_BOTTOM_TOP,120,COLOR_OUT0);
    scene->addItem(pipe7); // vertical pipe below valve V7
    point = pipe7->get_coordinate(COORD_BOTTOM);
    Pipe *pipe8 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(pipe8); // elbow below valve V7
    point = pipe8->get_coordinate(COORD_LEFT) + QPoint(-25,0);
    Pipe *pipe9 = new Pipe(point,PIPE2_LEFT_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe9); // horizontal pipe between V6 and V7
    point = pipe9->get_coordinate(COORD_LEFT) + QPoint(-34,-25);
    Meter *flow2 = new Meter(point,METER_HFLOW,"flow2");
    flow2->setError(false);
    flow2->setValue(0.0);
    point = flow2->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe4 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);
    scene->addItem(Tpipe4); // T-pipe at top of V6
    scene->addItem(flow2);  // First, add both pipes, then add flowmeter 2

    // Vertical pipe down to V6 and CFC
    point = Tpipe4->get_coordinate(COORD_BOTTOM) + QPoint(0,30);
    Valve *valve6 = new Valve(point,VERTICAL,"V6");
    scene->addItem(valve6); // valve V6
    point = valve6->get_coordinate(COORD_BOTTOM) + QPoint(0,20);
    Pipe *pipe11 = new Pipe(point,PIPE2_TOP_BOTTOM,40,COLOR_OUT0);
    scene->addItem(pipe11); // vertical pipe between V6 and CFC
    point = pipe11->get_coordinate(COORD_BOTTOM) + QPoint(-11,-1);
    Meter *flow3 = new Meter(point,METER_VFLOW,"flow3");
    flow3->setError(false);
    flow3->setValue(8.33333,5.1234);
    point = flow3->get_coordinate(COORD_BOTTOM) + QPoint(0,48);
    Pipe *pipex = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipex);
    scene->addItem(flow3);
    point += QPoint(50,-15);
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QGraphicsTextItem *text = scene->addText("To CFC (Counterflow Chiller)",font);
    text->setPos(point);

    // Continu going left from T-pipe at flowmeter 2 and Pump 1
    point = Tpipe4->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *pipe12 = new Pipe(point,PIPE2_LEFT_RIGHT,100,COLOR_OUT0);
    scene->addItem(pipe12); // horizontal pipe at left of Valve V6
    point = pipe12->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe3 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);                 // TODO: Pipe *Tpipe3 = new Pipe(point,PIPE3_NO_TOP,50,CONNECT_RIGHT_SIDE,pipe12)
    scene->addItem(Tpipe3); // First T-pipe at output of pump 1
    point = Tpipe3->get_coordinate(COORD_BOTTOM) + QPoint(0,50);

    // From T-pipe 13 going down to Pump 1
    Pipe *pipe14 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(pipe14); // elbow pipe right of pump 1
    point = pipe14->get_coordinate(COORD_LEFT) + QPoint(-60,20);
    Pump *pump1 = new Pump(point,OUT_RIGHT,"P1");
    pump1->setStatus(AUTO_OFF);
    scene->addItem(pump1); // Pump 1

    // Pump 1 left-input connection
    point.setX(Tpipe1->x()); // Elbow pipe pipe15 should have the x-coordinate of the T-pipe (pipe_in1) right of flowmeter 1
    point.setY(pump1->y());    // ... and the y-coordinate of pump 1
    Pipe *pipe15 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    scene->addItem(pipe15); // elbow pipe at  left (input) of pump 1

    // Now fit vertical pipe pipe16 between Tpipe1 and pipe15
    point = 0.5*(pipe15->get_coordinate(COORD_TOP) + Tpipe1->get_coordinate(COORD_BOTTOM)); // y-midpoint between both pipes
    len   = (pipe15->get_coordinate(COORD_TOP) - Tpipe1->get_coordinate(COORD_BOTTOM)).y();
    Pipe *pipe16 = new Pipe(point,PIPE2_TOP_BOTTOM,len,COLOR_IN0);
    scene->addItem(pipe16); // vertical pipe connecting pump input to main system

    // Now fit horizontal pipe pipe17 between elbow-pipe pipe15 and pump1
    point = 0.5*(pump1->get_coordinate(COORD_LEFT) + pipe15->get_coordinate(COORD_RIGHT)); // x-midpoint between pipe and pump
    len   = (pump1->get_coordinate(COORD_LEFT) - pipe15->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe17 = new Pipe(point,PIPE2_LEFT_RIGHT,len,COLOR_IN0);
    scene->addItem(pipe17); // horizontal pipe left of pump1

    // Connect HLT heat-exchanger output to MLT return-manifold
    point = hlt->get_coordinate(COORD_RIGHT_PIPE1) + QPoint(50,1); // get coordinates of top-right pipe of heat-exchanger
    Pipe *pipe18 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(pipe18); // elbow pipe right of HLT heat-exchanger output
    // Now calculate flowmeter 4, but don't add it yet
    point = pipe18->get_coordinate(COORD_TOP) + QPoint(-11,-47);
    Meter *flow4 = new Meter(point,METER_VFLOW,"flow4");
    flow4->setError(false);
    flow4->setValue(8.33333,5.1234);
    // Calculate size for top-left elbow, so that a separate vertical pipe is not needed
    point.setX(pipe18->x()); // set x-coordinate to lower elbow pipe
    point.setY(mlt->get_coordinate(COORD_LEFT_TOP_PIPE).y()); // get y-coordinate of top-left pipe for return-manifold in top
    int lenx = (mlt->get_coordinate(COORD_LEFT_TOP_PIPE) - flow4->get_coordinate(COORD_TOP)).x(); // x-diff between flowmeter and top elbow pipe
    int leny = (flow4->get_coordinate(COORD_TOP) - mlt->get_coordinate(COORD_LEFT_TOP_PIPE)).y(); // y-diff between flowmeter and top elbow pipe
    len = qMax(lenx,leny);
    Pipe *pipe19 = new Pipe(point,PIPE2_BOTTOM_RIGHT,len,COLOR_OUT0);
    scene->addItem(pipe19); // elbow pipe left of MLT return-manifold input
    scene->addItem(flow4);  // add flow4 after pipes have been added

    // From T-pipe 3 going left to HLT heat-exchanger input
    point = Tpipe3->get_coordinate(COORD_LEFT) + QPoint(-75,0);
    Pipe *pipe20 = new Pipe(point,PIPE2_RIGHT_LEFT,150,COLOR_OUT0);
    scene->addItem(pipe20); // horizontal pipe at left of Valve V6
    point = pipe20->get_coordinate(COORD_LEFT) + QPoint(-25,0);
    Pipe *pipe21 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe21); // elbow pipe at output, left of T-pipe 13
    point.setY(pipe2->y()); // set Y-coordinate to same value as elbow pipe 2
    Pipe *pipe22 = new Pipe(point,PIPE2_LEFT_BOTTOM,50,COLOR_OUT0);
    scene->addItem(pipe22); // elbow pipe connecting to lower pipe of HLT heat-exchanger

    // Add Valve V4 with same y-coordinates as other valves, but with x-coordinate from pipe21/pipe22
    point.setY(valve1->y());
    Valve *valve4 = new Valve(point,VERTICAL,"V4");
    scene->addItem(valve4); // valve V4

    // Connect elbow pipe 21 to valve V4 bottom
    point = 0.5*(pipe21->get_coordinate(COORD_TOP) + valve4->get_coordinate(COORD_BOTTOM)); // y-midpoint between pipe 21 and valve 4
    len   = (pipe21->get_coordinate(COORD_TOP) - valve4->get_coordinate(COORD_BOTTOM)).y(); // y-length
    Pipe *pipe23 = new Pipe(point,PIPE2_BOTTOM_TOP_NO_ARROW,len+1,COLOR_OUT0);
    scene->addItem(pipe23);

    // Connect top op Valve V4 to elbow pipe 21 (that connects to lower pipe of HLT heat-exchanger)
    point = 0.5*(valve4->get_coordinate(COORD_TOP) + pipe22->get_coordinate(COORD_BOTTOM)); // y-midpoint between valve 4 and pipe 22
    len   = (valve4->get_coordinate(COORD_TOP) - pipe22->get_coordinate(COORD_BOTTOM)).y(); // y-length
    Pipe *pipe24 = new Pipe(point,PIPE2_BOTTOM_TOP,len,COLOR_OUT0);
    scene->addItem(pipe24);

    // Now connect lower pipe of HLT heat-exchanger to elbow pipe 22
    point = 0.5*(pipe22->get_coordinate(COORD_LEFT) + hlt->get_coordinate(COORD_RIGHT_PIPE2)); // x-midpoint between hlt pipe-output and pipe 22
    len   = (pipe22->get_coordinate(COORD_LEFT) - hlt->get_coordinate(COORD_RIGHT_PIPE2)).x(); // x-length
    Pipe *pipe25 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_OUT0);
    scene->addItem(pipe25);

    //Tpipe4->setColor(COLOR_OUT1);
    //Tpipe2->setColor(COLOR_IN1);
} // draw_hmi_screen()

/*-----------------------------------------------------------------------------
  Purpose    : Main-entry program of Ebrew v3 Qt.
  Variables  : argc: argument count
               argv: pointer to arguments
  Returns    : 0 if no error
  ---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    draw_hmi_screen(&scene); // Draw the total Human-Machine Interface on screen
    view.showNormal();
    view.fitInView(scene.sceneRect().adjusted(-20, -50, 20, 50), Qt::KeepAspectRatio);

    return app.exec();
}
