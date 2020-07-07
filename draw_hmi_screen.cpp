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
#include <QMainWindow>
#include <QMenuBar>
#include <QVBoxLayout>
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

    //----------------------------------------------------------
    // Draw HLT tank with Pump 2
    // Objects: hlt, pump2, elbow0, elbow1, pipe2, pipe3, pipe1
    //----------------------------------------------------------
    Tank *hlt = new Tank(-700,0,250,300,TANK_EXIT_BOTTOM|TANK_HEAT_EXCHANGER,"HLT 200L");
    scene->addItem(hlt); // Hot-Liquid Tun (HLT)
    p->hlt = hlt;        // save hlt reference to MainEbrew
    hlt->setValues(21.3,65.5,121.5,100.0); // TODO: remove
    point = hlt->get_coordinate(COORD_LEFT_PIPE1) + QPoint(-60,20); // get coordinates of top-left pipe of heat-exchanger
    Pump *pump2 = new Pump(point,OUT_RIGHT,"P2");
    pump2->setStatus(AUTO_OFF);
    scene->addItem(pump2);
    p->P2 = pump2;          // add pump2 reference to MainEbrew
    point = pump2->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *elbow0 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_IN0);
    scene->addItem(elbow0); // elbow pipe at left of pump 2
    point = elbow0->get_coordinate(COORD_BOTTOM) + QPoint(0,50);
    Pipe *elbow1 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    scene->addItem(elbow1); // elbow pipe bottom-left of pump 2

    point = hlt->get_coordinate(COORD_LEFT_PIPE2) + QPoint(-6,0); // get coordinates of bottom-left pipe of heat-exchanger
    Pipe *pipe2 = new Pipe(point,PIPE2_BOTTOM_RIGHT,50,COLOR_IN0);
    scene->addItem(pipe2);
    point = pipe2->get_coordinate(COORD_BOTTOM) + QPoint(0,25);
    point.setY(elbow1->y()); // same y coordinate as pipe 1
    Pipe *pipe3 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_IN0);
    scene->addItem(pipe3);   // elbow-pipe bottom-right of pump 2
    point = 0.5*(elbow1->get_coordinate(COORD_RIGHT) + pipe3->get_coordinate(COORD_LEFT)); // x-midpoint between pipe1 and pipe 3
    len   = (pipe3->get_coordinate(COORD_LEFT) - elbow1->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe1 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    scene->addItem(pipe1);   // connecting pipe below pump 2

    //----------------------------------------------------------
    // Draw HLT output pipe with V2
    // Objects: valve2, elbow2, flow1, Tpipe1
    //----------------------------------------------------------
    point = hlt->get_coordinate(COORD_BOTTOM_PIPE1) + QPoint(-RPIPE,30); // get coordinates of bottom-left pipe
    Valve *valve2 = new Valve(point,VERTICAL,"V2");
    scene->addItem(valve2);
    p->V2 = valve2;          // Add valve2 reference to MainEbrew
    point = valve2->get_coordinate(COORD_BOTTOM) + QPoint(0,50);
    Pipe *elbow2 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    scene->addItem(elbow2);  // elbow pipe below valve V2
    point = elbow2->get_coordinate(COORD_RIGHT) + QPoint(12,-25);
    Meter *flow1 = new Meter(point,METER_HFLOW,"flow1");
    flow1->setError(false);  // Init flowmeter 1
    flow1->setValue(0.0);
    p->F1 = flow1;           // Add flow1 reference to MainEbrew
    point = flow1->get_coordinate(COORD_RIGHT) + QPoint(50,1);
    Pipe *Tpipe1 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_IN0); // T-pipe right of flowmeter 1
    scene->addItem(Tpipe1);  // First, draw both pipes, then add flowmeter!
    scene->addItem(flow1);   // Flowmeter between HLT and Pump 1 input

    //----------------------------------------------------------
    // Draw MLT tank with output pipe and valve V1
    // Objects: mlt, valve1, Tpipe2, pipe4
    //----------------------------------------------------------
    Tank *mlt = new Tank(-270,0,250,300,TANK_MANIFOLD_BOTTOM|TANK_MANIFOLD_TOP,"MLT 110L");
    scene->addItem(mlt);     // Mash-Lauter Tun (MLT)
    p->mlt = mlt;            // add MLT reference to MainEbrew
    point = mlt->get_coordinate(COORD_BOTTOM_PIPE1); // get coordinates of bottom-left pipe
    point += QPoint(-RPIPE,30);
    Valve *valve1 = new Valve(point,VERTICAL,"V1");
    scene->addItem(valve1);  // valve V1
    p->V1 = valve1;          // add valve1 reference to MainEbrew
    point += QPoint(0,80);
    Pipe *Tpipe2 = new Pipe(point,PIPE3_NO_BOTTOM,50,COLOR_IN0);
    scene->addItem(Tpipe2);  // T-pipe below valve V1
    point = 0.5*(Tpipe2->get_coordinate(COORD_LEFT) + Tpipe1->get_coordinate(COORD_RIGHT)); // x-midpoint between Tpipe1 and pipe 5
    len   = (Tpipe2->get_coordinate(COORD_LEFT) - Tpipe1->get_coordinate(COORD_RIGHT)).x(); // x-length
    Pipe *pipe4 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    scene->addItem(pipe4);   // horizontal connecting pipe between HLT and MLT

    //----------------------------------------------------------
    // Draw boil-kettle with output pipe and valve V3
    // Objects: boil, valve3, elbow3, pipe5
    //----------------------------------------------------------
    Tank *boil = new Tank(60,0,275,300,TANK_MANIFOLD_BOTTOM|TANK_RETURN_BOTTOM,"BOIL 140L");
    scene->addItem(boil);
    p->boil = boil;          // add boil reference to MainEbrew
    point = boil->get_coordinate(COORD_BOTTOM_PIPE1) + QPoint(-RPIPE,30); // get coordinates of bottom-left pipe
    Valve *valve3 = new Valve(point,VERTICAL,"V3");
    scene->addItem(valve3);  // Valve V3 at input-side of pump
    p->V3 = valve3;          // add valve3 reference to MainEbrew
    point += QPoint(0,80);
    Pipe *elbow3 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_IN0);
    scene->addItem(elbow3);  // elbow pipe below valve V3, needs to be connected to pipe4 (T-pipe below valve V1)
    point = 0.5*(elbow3->get_coordinate(COORD_LEFT) + Tpipe2->get_coordinate(COORD_RIGHT)); // x-midpoint between pipe 5 and pipe 6
    len   = (elbow3->get_coordinate(COORD_LEFT) - Tpipe2->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe5 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_IN0);
    scene->addItem(pipe5);   // horizontal connecting pipe between MLT and Boil-kettle

    //----------------------------------------------------------
    // Draw boil-kettle input pipe with valve V7
    // Objects: valve7, pipe6, elbow4, pipe7, flow2, Tpipe4
    //----------------------------------------------------------
    point = boil->get_coordinate(COORD_BOTTOM_PIPE2) + QPoint(-RPIPE,30); // get coordinates of bottom-right pipe
    Valve *valve7 = new Valve(point,VERTICAL,"V7");
    scene->addItem(valve7);  // valve V7
    p->V7 = valve7;          // add valve7 reference to MainEbrew
    point = valve7->get_coordinate(COORD_BOTTOM) + QPoint(0,60);
    Pipe *pipe6 = new Pipe(point,PIPE2_BOTTOM_TOP,120,COLOR_OUT0);
    scene->addItem(pipe6);   // vertical pipe below valve V7
    point = pipe6->get_coordinate(COORD_BOTTOM);
    Pipe *elbow4 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(elbow4);  // elbow below valve V7
    point = elbow4->get_coordinate(COORD_LEFT) + QPoint(-25,0);
    Pipe *pipe7 = new Pipe(point,PIPE2_LEFT_RIGHT,50,COLOR_OUT0);
    scene->addItem(pipe7);   // horizontal pipe between V6 and V7
    point = pipe7->get_coordinate(COORD_LEFT) + QPoint(-34,-25);
    Meter *flow2 = new Meter(point,METER_HFLOW,"flow2");
    flow2->setError(false);
    flow2->setValue(0.0);
    p->F2 = flow2;           // Add flow2 reference to MainEbrew
    point = flow2->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe4 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);
    scene->addItem(Tpipe4);  // T-pipe at top of V6
    scene->addItem(flow2);   // First, add both pipes, then add flowmeter 2

    //----------------------------------------------------------
    // Draw vertical pipe down to V6 and CFC
    // Objects: valve6, pipe8, flow3, elbow5
    //----------------------------------------------------------
    point = Tpipe4->get_coordinate(COORD_BOTTOM) + QPoint(0,30);
    Valve *valve6 = new Valve(point,VERTICAL,"V6");
    scene->addItem(valve6);  // valve V6
    p->V6 = valve6;          // add valve6 reference to MainEbrew
    point = valve6->get_coordinate(COORD_BOTTOM) + QPoint(0,20);
    Pipe *pipe8 = new Pipe(point,PIPE2_TOP_BOTTOM,40,COLOR_OUT0);
    scene->addItem(pipe8);   // vertical pipe between V6 and CFC
    point = pipe8->get_coordinate(COORD_BOTTOM) + QPoint(-11,-1);
    Meter *flow3 = new Meter(point,METER_VFLOW,"flow3");
    flow3->setError(false);
    flow3->setValue(8.33333,5.1234);
    point = flow3->get_coordinate(COORD_BOTTOM) + QPoint(0,48);
    Pipe *elbow5 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    scene->addItem(elbow5);  // Exit pipe from flow3 to CFC
    scene->addItem(flow3);
    p->F3 = flow3;           // Add flow3 reference to MainEbrew
    point += QPoint(50,-15);
    font.setPointSize(12);
    font.setBold(true);
    QGraphicsTextItem *text = scene->addText("To CFC (Counterflow Chiller)",font);
    text->setPos(point);     // insert text

    //----------------------------------------------------------
    // Draw left from T-pipe (Tpipe4) and left from valve V6
    // Objects: pipe9, Tpipe3
    //----------------------------------------------------------
    point = Tpipe4->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *pipe9 = new Pipe(point,PIPE2_LEFT_RIGHT,100,COLOR_OUT0);
    scene->addItem(pipe9);   // horizontal pipe at left of Valve V6
    point = pipe9->get_coordinate(COORD_LEFT) + QPoint(-50,0);
    Pipe *Tpipe3 = new Pipe(point,PIPE3_NO_TOP,50,COLOR_OUT0);
    scene->addItem(Tpipe3);  // First T-pipe at output of pump 1
    point = Tpipe3->get_coordinate(COORD_BOTTOM) + QPoint(0,50);

    //----------------------------------------------------------
    // Draw from T-pipe (Tpipe3) down to Pump 1
    // Objects: elbow6, pump1
    //----------------------------------------------------------
    Pipe *elbow6 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(elbow6);  // elbow pipe right of pump 1
    point = elbow6->get_coordinate(COORD_LEFT) + QPoint(-60,20);
    Pump *pump1 = new Pump(point,OUT_RIGHT,"P1");
    pump1->setStatus(AUTO_OFF);
    scene->addItem(pump1);   // Pump 1
    p->P1 = pump1;           // Add pump1 reference to MainEbrew

    //----------------------------------------------------------
    // Draw left-input connection of Pump 1
    // Objects: elbow7, pipe10, pipe11
    //----------------------------------------------------------
    point.setX(Tpipe1->x()); // Elbow pipe elbow7 should have the x-coordinate of the T-pipe (Tpipe1) right of flowmeter 1
    point.setY(pump1->y());  // ... and the y-coordinate of pump 1
    Pipe *elbow7 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_IN0);
    scene->addItem(elbow7);  // elbow pipe at  left (input) of pump 1

    // Now fit vertical pipe pipe1 between Tpipe1 and elbow7
    point = 0.5*(elbow7->get_coordinate(COORD_TOP) + Tpipe1->get_coordinate(COORD_BOTTOM)); // y-midpoint between both pipes
    len   = (elbow7->get_coordinate(COORD_TOP) - Tpipe1->get_coordinate(COORD_BOTTOM)).y();
    Pipe *pipe10 = new Pipe(point,PIPE2_TOP_BOTTOM,len,COLOR_IN0);
    scene->addItem(pipe10);  // vertical pipe connecting pump input to main system

    // Now fit horizontal pipe pipe11 between elbow-pipe elbow7 and pump1
    point = 0.5*(pump1->get_coordinate(COORD_LEFT) + elbow7->get_coordinate(COORD_RIGHT)); // x-midpoint between pipe and pump
    len   = (pump1->get_coordinate(COORD_LEFT) - elbow7->get_coordinate(COORD_RIGHT)).x();
    Pipe *pipe11 = new Pipe(point,PIPE2_LEFT_RIGHT,len,COLOR_IN0);
    scene->addItem(pipe11);  // horizontal pipe left of pump1

    //----------------------------------------------------------
    // Connect HLT heat-exchanger output to MLT return-manifold
    // Objects: elbow8, flow4, elbow9
    //----------------------------------------------------------
    point = hlt->get_coordinate(COORD_RIGHT_PIPE1) + QPoint(50,1); // get coordinates of top-right pipe of heat-exchanger
    Pipe *elbow8 = new Pipe(point,PIPE2_LEFT_TOP,50,COLOR_OUT0);
    scene->addItem(elbow8);  // lower elbow pipe right of HLT heat-exchanger output
    // Now calculate flowmeter 4 position, but don't add it yet
    point = elbow8->get_coordinate(COORD_TOP) + QPoint(-11,-47);
    Meter *flow4 = new Meter(point,METER_VFLOW,"flow4");
    flow4->setError(false);
    flow4->setValue(8.33333,5.1234);
    // Calculate size for top-left elbow, so that a separate vertical pipe is not needed
    point.setX(elbow8->x()); // set x-coordinate to lower elbow pipe
    point.setY(mlt->get_coordinate(COORD_LEFT_TOP_PIPE).y()); // get y-coordinate of top-left pipe for return-manifold in top
    int lenx = (mlt->get_coordinate(COORD_LEFT_TOP_PIPE) - flow4->get_coordinate(COORD_TOP)).x(); // x-diff between flowmeter and top elbow pipe
    int leny = (flow4->get_coordinate(COORD_TOP) - mlt->get_coordinate(COORD_LEFT_TOP_PIPE)).y(); // y-diff between flowmeter and top elbow pipe
    len = qMax(lenx,leny);   // use the larger of the two
    Pipe *elbow9 = new Pipe(point,PIPE2_BOTTOM_RIGHT,len,COLOR_OUT0);
    scene->addItem(elbow9);  // top elbow pipe, left of MLT return-manifold input
    scene->addItem(flow4);   // add flow4 after pipes have been added
    p->F4 = flow4;           // add flow4 reference to MainEbrew

    //----------------------------------------------------------
    // From T-pipe 3 going left to HLT heat-exchanger input
    // Objects: pipe12, elbow10, elbow11
    //----------------------------------------------------------
    point = Tpipe3->get_coordinate(COORD_LEFT) + QPoint(-75,0);
    Pipe *pipe12 = new Pipe(point,PIPE2_RIGHT_LEFT,150,COLOR_OUT0);
    scene->addItem(pipe12);  // horizontal pipe left of Tpipe3 at Pump 1 output
    point = pipe12->get_coordinate(COORD_LEFT) + QPoint(-25,0);
    Pipe *elbow10 = new Pipe(point,PIPE2_TOP_RIGHT,50,COLOR_OUT0);
    scene->addItem(elbow10); // elbow pipe at pump 1 output, left of Tpipe3
    point.setY(pipe2->y());  // set Y-coordinate to same value as elbow pipe 2
    Pipe *elbow11 = new Pipe(point,PIPE2_LEFT_BOTTOM,50,COLOR_OUT0);
    scene->addItem(elbow11); // elbow pipe connecting to lower pipe of HLT heat-exchanger

    //-------------------------------------------------------------------------------------------------
    // Connect Valve V4 to pump output (elbow10) and to HLT heat-exchanger input (elbow11)
    // V4 should have same y-coordinate as other valves, but with x-coordinate from elbow10/elbow11
    // Objects: valve4, pipe13, pipe14, elbow12
    //-------------------------------------------------------------------------------------------------
    point.setY(valve1->y()); // get y-coordinate from one of the valves
    Valve *valve4 = new Valve(point,VERTICAL,"V4");
    scene->addItem(valve4);  // valve V4
    p->V4 = valve4;          // add valve4 reference to MainEbrew

    // Connect elbow pipe 10 to bottom of valve V4
    point = 0.5*(elbow10->get_coordinate(COORD_TOP) + valve4->get_coordinate(COORD_BOTTOM)); // y-midpoint between pipe 21 and valve 4
    len   = (elbow10->get_coordinate(COORD_TOP) - valve4->get_coordinate(COORD_BOTTOM)).y(); // y-length
    Pipe *pipe13 = new Pipe(point,PIPE2_BOTTOM_TOP_NO_ARROW,len+1,COLOR_OUT0);
    scene->addItem(pipe13);

    // Connect top of Valve V4 to elbow pipe 11 (that connects to lower pipe of HLT heat-exchanger)
    point = 0.5*(valve4->get_coordinate(COORD_TOP) + elbow11->get_coordinate(COORD_BOTTOM)); // y-midpoint between valve 4 and pipe 22
    len   = (valve4->get_coordinate(COORD_TOP) - elbow11->get_coordinate(COORD_BOTTOM)).y(); // y-length
    Pipe *pipe14 = new Pipe(point,PIPE2_BOTTOM_TOP,len,COLOR_OUT0);
    scene->addItem(pipe14);

    // Now connect lower pipe of HLT heat-exchanger to elbow pipe 11
    point = 0.5*(elbow11->get_coordinate(COORD_LEFT) + hlt->get_coordinate(COORD_RIGHT_PIPE2)); // x-midpoint between hlt pipe-output and pipe 22
    len   = (elbow11->get_coordinate(COORD_LEFT) - hlt->get_coordinate(COORD_RIGHT_PIPE2)).x(); // x-length
    Pipe *elbow12 = new Pipe(point,PIPE2_RIGHT_LEFT,len,COLOR_OUT0);
    scene->addItem(elbow12);
} // draw_hmi_screen()


/*-----------------------------------------------------------------------------
  Purpose    : Main-entry program of Ebrew v3 Qt.
  Variables  : argc: argument count
               argv: pointer to arguments
  Returns    : 0 if no error
  ---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    QApplication   app(argc, argv);
    QGraphicsScene scene;
    QGraphicsView  view(&scene);

    auto mainWindow = new QMainWindow;
    auto Ebrew      = new MainEbrew;

    auto menuBar    = new QMenuBar;
    auto menu       = new QMenu("&File");
    auto action     = new QAction("Read Log-File...");
    menu->addAction(action);
    menuBar->addMenu(menu);
    mainWindow->setMenuBar(menuBar);

    auto frame = new QFrame;
    frame->setLayout(new QVBoxLayout);
    mainWindow->setCentralWidget(frame);

    app.setApplicationName("Ebrew 3.0 Qt: Automating your Home-Brewery!");
    draw_hmi_screen(&scene,Ebrew); // Draw the total Human-Machine Interface on screen
    view.showNormal();
    view.fitInView(scene.sceneRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);

    frame->layout()->addWidget(&view);

    mainWindow->setFixedSize(1100,900);
    mainWindow->showNormal();
    return app.exec();
} // main()
