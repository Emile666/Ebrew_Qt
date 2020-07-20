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
#include <QVBoxLayout>
#include <QThread>
#include <QTimer>
#include <QLabel>
#include "hmi_objects.h"
#include "MainEbrew.h"
#include "draw_hmi_screen.h"
#include "scheduler.h"

/*-----------------------------------------------------------------------------
  Purpose    : Main-entry program of Ebrew v3 Qt.
  Variables  : argc: argument count
               argv: pointer to arguments
  Returns    : 0 if no error
  ---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    QApplication   app(argc, argv);    // Qt main application
    QGraphicsScene scene;              // Container for all graphical objects
    QGraphicsView  view(&scene);       // Viewer for all graphical objects

    Q_INIT_RESOURCE(icons);

    //------------------------------------------------------
    // Initialises the EBrew object. It does the following:
    // - Create the statusbar at the bottom of the screen
    // - Create the menu-bar with all signals and slots
    // - Init. the Registry or create a default one if not found
    // - Read the Mash Scheme file
    //------------------------------------------------------
    auto Ebrew = new MainEbrew;   // Brew controller for HERMS system (QMainWindow)

    //------------------------------------------------------
    // Create a new thread with a Timer and a scheduler
    // for all time-related Ebrew processes
    //------------------------------------------------------
    QThread   *mainThread = new QThread;
    QTimer    *mainTimer  = new QTimer(0); // 0 is needed here
    Scheduler *scheduler  = new Scheduler();

    mainTimer->setInterval(100); // set to 100 msec.
    mainTimer->moveToThread(mainThread);
    QObject::connect(mainTimer,SIGNAL(timeout()),Ebrew,SLOT(T100msecLoop()));
    // Make sure the timer gets started from mainThread.
    mainTimer->connect(mainThread,SIGNAL(started()),SLOT(start()));
    mainThread->start(); // start the thread

    Ebrew->connect(mainTimer,SIGNAL(timeout()),scheduler,SLOT(scheduler_isr()));  // runs at 100 msec.
    Ebrew->connect(mainTimer,SIGNAL(timeout()),scheduler,SLOT(dispatch_tasks())); // dispatcher also runs at 100 msec.
    scheduler->add_task("Alive_led" ,  0, 500,Ebrew,SLOT(task_alive_led()));      // TASK 0
    scheduler->add_task("read_temps",100,2000,Ebrew,SLOT(task_read_temps()));     // TASK 1
    scheduler->add_task("update_std",400,1000,Ebrew,SLOT(task_update_std()));     // TASK 3
    scheduler->add_task("read_flows",600,2000,Ebrew,SLOT(task_read_flows()));     // TASK 5

    //------------------------------------------------------
    // Create the HMI screen and connect it to Ebrew
    //------------------------------------------------------
    auto frame = new QFrame;
    frame->setLayout(new QVBoxLayout);
    Ebrew->setCentralWidget(frame);
    app.setApplicationName("Ebrew 3.0 Qt: Automating your Home-Brewery!");
    draw_hmi_screen(&scene,Ebrew); // Draw the total Human-Machine Interface on screen

    view.showNormal();
    view.fitInView(scene.sceneRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
    frame->layout()->addWidget(&view);
    Ebrew->setFixedSize(1100,900);
    Ebrew->showNormal();
    scheduler->start();  // start scheduler if everything is initialized

    return app.exec();
} // main()
