/**************************************************************************************
** Filename    : scheduler.h
** Author      : Emile
** Purpose     : Header file for scheduler.cpp. The scheduler is an object for
**               non pre-emptive task-switching.
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
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <QObject>

#define MAX_TASKS        (10) /* Max. number of tasks */
#define MAX_MSEC      (60000) /* Max. period time in msec. for a task */
#define TICKS_PER_SEC   (10L) /* Nr. of calls to scheduler_isr() in one second */
#define NAME_LEN         (30) /* Max. name length of task name */

#define TASK_READY    (0x01)
#define TASK_ENABLED  (0x02)

#define NO_ERR        (0x00)
#define ERR_NAME      (0x01)
#define ERR_EMPTY     (0x02)
#define ERR_MAX_TASKS (0x03)

typedef struct _task_struct
{
    char     Name[NAME_LEN];      // Task name
    uint16_t Period;              // Period between 2 calls in msec.
    uint16_t Delay;               // Initial delay before Counter starts in msec.
    uint16_t Counter;             // Running counter, is init. from Period
	uint8_t	 Status;              // bit 1: 1=enabled ; bit 0: 1=ready to run
    uint32_t Duration;            // Measured task-duration in usec.
	uint32_t Duration_Max;        // Max. measured task-duration
} task_struct;

class Scheduler : public QObject
{
	Q_OBJECT

signals: // functions that emit signal for a specific task
    void    signal_task0(void);
    void    signal_task1(void);
    void    signal_task2(void);
    void    signal_task3(void);
    void    signal_task4(void);
    void    signal_task5(void);
    void    signal_task6(void);
    void    signal_task7(void);
    void    signal_task8(void);
    void    signal_task9(void);

public slots:
    void    scheduler_isr(void);  // run-time function for scheduler
    void    dispatch_tasks(void); // run all tasks that are ready

public:
	Scheduler(void);
    uint8_t add_task(const char *Name, uint16_t delay, uint16_t period, const QObject *receiver,const char *method);
    uint8_t set_task_time_period(quint16 Period, const char *Name);
	uint8_t enable_task(char *Name);
	uint8_t disable_task(char *Name);
    uint8_t updateDuration(const char *Name, uint16_t duration); // Needed for signal/slot workaround
    QString list_all_tasks(void);
    void    start(void);              // start running the scheduler
    void    stop(void);               // stop running the scheduler
protected:
	task_struct task_list[MAX_TASKS]; // struct with all tasks
    uint8_t     max_tasks;            // actual number of tasks added
    bool        started;              // true = scheduler is running
}; // class Scheduler

#endif
