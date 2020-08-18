/**************************************************************************************
** Filename    : main.cpp
** Author      : Emile
** Purpose     : This is the main program-entry for the Ebrew PC-program.
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
#include <QApplication>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QThread>
#include <QTimer>
#include <QLabel>
#include <QDebug>

#include "hmi_objects.h"
#include "MainEbrew.h"
#include "draw_hmi_screen.h"
#include "scheduler.h"

// NOTE add hop-duration timers to mash-scheme file

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
    // - Init. Brew-day settings
    // - Init. slope-limiters
    //------------------------------------------------------
    auto Ebrew = new MainEbrew;   // Brew controller for HERMS system (QMainWindow)

    //------------------------------------------------------
    // Create a new thread with a Timer and a scheduler
    // for all time-related Ebrew processes
    //------------------------------------------------------
    QThread   *mainThread = new QThread;
    QTimer    *mainTimer  = new QTimer(0); // 0 is needed here
    Scheduler *scheduler  = new Scheduler();

    mainTimer->setTimerType(Qt::PreciseTimer); // set to millisecond accuracy
    mainTimer->setInterval(100); // set to 100 msec.
    mainTimer->moveToThread(mainThread);
    // Make sure the timer gets started from mainThread.
    mainTimer->connect(mainThread,SIGNAL(started()),SLOT(start()));
    mainThread->start(); // start the thread

    Ebrew->connect(mainTimer,SIGNAL(timeout()),scheduler,SLOT(scheduler_isr()));  // runs at 100 msec.
    Ebrew->connect(mainTimer,SIGNAL(timeout()),scheduler,SLOT(dispatch_tasks())); // dispatcher also runs at 100 msec.
    scheduler->add_task("aliveLed"  ,   0,TS_LED_MSEC  ,Ebrew,SLOT(task_alive_led()));      // TASK 0
    scheduler->add_task("readTemps" , 100,TS_TEMPS_MSEC,Ebrew,SLOT(task_read_temps()));     // TASK 1
    scheduler->add_task("pidControl", 300,TS_PID_MSEC  ,Ebrew,SLOT(task_pid_control()));    // TASK 2
    scheduler->add_task("updateStd" , 400,TS_STD_MSEC  ,Ebrew,SLOT(task_update_std()));     // TASK 3
    scheduler->add_task("readFlows" , 600,TS_FLOWS_MSEC,Ebrew,SLOT(task_read_flows()));     // TASK 5
    scheduler->add_task("wrLogFile" ,1600,TS_WR_LOGFILE,Ebrew,SLOT(task_write_logfile()));  // TASK 6
    Ebrew->schedulerEbrew = scheduler; // copy pointer to scheduler to MainEbrew

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
    Ebrew->setFixedSize(scene.sceneRect().width(),scene.sceneRect().height());
    //Ebrew->setFixedSize(1300,900);
    //Ebrew->showNormal();
    Ebrew->showMaximized();
    scheduler->start();  // start scheduler if everything is initialized

    return app.exec();
} // main()
