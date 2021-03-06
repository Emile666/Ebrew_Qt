/**************************************************************************************
** Filename    : scheduler.cpp
** Author      : Emile
** Purpose     : This file contains the scheduler object for non pre-emptive task-switching.
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
#include <QElapsedTimer>
#include "scheduler.h"

Scheduler::Scheduler(void) : QObject()
{
	memset(task_list,0x00,sizeof(task_list)); // clear task_list array
    max_tasks = 0;
    started   = false;
} // Scheduler::Scheduler()

void Scheduler::start(void)
{
     started = true; // start running the scheduler
} // Scheduler::start()

void Scheduler::stop(void)
{
    started = false; // stop running the scheduler
} // Scheduler::stop()

/*-----------------------------------------------------------------------------
  Purpose  : Run-time function for scheduler. Should be called from within
             an ISR. This function goes through the task-list and decrements
             eacht task counter. On time-out, the ready flag is set.
  Variables: task_list[] structure
  Returns  : -
  ---------------------------------------------------------------------------*/
void Scheduler::scheduler_isr(void)
{
	uint8_t index = 0; // index in task_list struct

    while (started && task_list[index].Period)
	{
		//First go through the initial delay
		if(task_list[index].Delay > 0)
		{
			task_list[index].Delay--;
		} // if
		else
		{	//now we decrement the actual period counter 
			task_list[index].Counter--;
			if(task_list[index].Counter == 0)
			{
				//Set the flag and reset the counter;
				task_list[index].Status |= TASK_READY;
			} // if
		} // else
		index++;
	} // while
} // Scheduler::scheduler_isr()

/*-----------------------------------------------------------------------------
  Purpose  : Run all tasks for which the ready flag is set. Should be called 
             from within the main() function, not an interrupt routine!
  Variables: task_list[] structure
  Returns  : -
  ---------------------------------------------------------------------------*/
void Scheduler::dispatch_tasks(void)
{
    uint8_t index = 0;
    //QElapsedTimer timer;

    //go through the active tasks
    while (started && task_list[index].Period)
    {
        if((task_list[index].Status & (TASK_READY | TASK_ENABLED)) == (TASK_READY | TASK_ENABLED))
        {
            //timer.start(); // start the elapsed timer
            switch (index)
            {   // send signal for specific task
                case 0 : emit signal_task0(); break;
                case 1 : emit signal_task1(); break;
                case 2 : emit signal_task2(); break;
                case 3 : emit signal_task3(); break;
                case 4 : emit signal_task4(); break;
                case 5 : emit signal_task5(); break;
                case 6 : emit signal_task6(); break;
                case 7 : emit signal_task7(); break;
                case 8 : emit signal_task8(); break;
                case 9 : emit signal_task9(); break;
            } // switch
            task_list[index].Status &= ~TASK_READY; // reset the task when finished
            task_list[index].Counter = task_list[index].Period; // reset counter
//            task_list[index].Duration = (uint16_t)(timer.nsecsElapsed()); // duration in milliseconds
//            if (task_list[index].Duration > task_list[index].Duration_Max)
//            {   // determine max. duration
//                task_list[index].Duration_Max = task_list[index].Duration;
//            } // if
        } // if
        index++; // next task
    } // while
    //go to sleep till next tick!
} // Scheduler::dispatch_tasks()

/*-----------------------------------------------------------------------------
  Purpose  : Add a function to the task-list struct. Should be called upon
  		     initialization.
  Variables: task_ptr: pointer to function
             delay   : initial delay in msec.
             period  : period between two calls in msec.
             receiver: pointer to the receiving object for the SLOT method
             method  : the method that will be executed
  Returns  : [NO_ERR, ERR_MAX_TASKS]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::add_task(const char *Name, uint16_t delay, uint16_t period, const QObject *receiver,const char *method)
{
    uint8_t  index = 0;
    uint16_t temp1 = (uint16_t)(delay  * TICKS_PER_SEC / 1000);
    uint16_t temp2 = (uint16_t)(period * TICKS_PER_SEC / 1000);

    if (max_tasks >= MAX_TASKS) return ERR_MAX_TASKS;

    if(task_list[index].Period != 0)
    {   //go through the active tasks
        while(task_list[++index].Period != 0) ;
    } // if

    if(task_list[index].Period == 0)
    {   // at end of task list and max_tasks < MAX_TASKS
        task_list[index].Period       = temp2;          // Period in msec.
        task_list[index].Counter      = temp2;	        // Countdown timer
        task_list[index].Delay        = temp1;          // Initial delay before start
        task_list[index].Status      |= TASK_ENABLED;   // Eable task by default
        task_list[index].Status      &= ~TASK_READY;    // Task not ready to run
        task_list[index].Duration     = 0;              // Actual Task Duration
        task_list[index].Duration_Max = 0;              // Max. Task Duration
        strncpy(task_list[index].Name, Name, NAME_LEN); // Name of Task
        max_tasks++; // increase number of tasks
        switch (index)
        {
            case 0: connect(this,SIGNAL(signal_task0()),receiver,method); break;
            case 1: connect(this,SIGNAL(signal_task1()),receiver,method); break;
            case 2: connect(this,SIGNAL(signal_task2()),receiver,method); break;
            case 3: connect(this,SIGNAL(signal_task3()),receiver,method); break;
            case 4: connect(this,SIGNAL(signal_task4()),receiver,method); break;
            case 5: connect(this,SIGNAL(signal_task5()),receiver,method); break;
            case 6: connect(this,SIGNAL(signal_task6()),receiver,method); break;
            case 7: connect(this,SIGNAL(signal_task7()),receiver,method); break;
            case 8: connect(this,SIGNAL(signal_task8()),receiver,method); break;
            case 9: connect(this,SIGNAL(signal_task9()),receiver,method); break;
            default: break;
        } // switch
    } // if
    return NO_ERR;
} // Scheduler::add_task()

/*-----------------------------------------------------------------------------
  Purpose  : Enable a task.
  Variables: Name: Name of task to enable
  Returns  : error [NO_ERR, ERR_NAME, ERR_EMPTY]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::enable_task(char *Name)
{
	uint8_t index = 0;
    uint8_t found = false;

	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Status |= TASK_ENABLED;
                found = true;
			} // if
			index++;
		} // while
	} // if
	else return ERR_EMPTY;
	if (!found)
	     return ERR_NAME;
	else return NO_ERR;
} // Scheduler::enable_task()

/*-----------------------------------------------------------------------------
  Purpose  : Manually update a duration for a task.
  Variables: Name: Name of task to enable
  Returns  : error [NO_ERR, ERR_NAME, ERR_EMPTY]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::updateDuration(const char *Name, uint16_t duration)
{
    uint8_t index = 0;
    uint8_t found = false;

    //go through the active tasks
    if(task_list[index].Period != 0)
    {
        while ((task_list[index].Period != 0) && !found)
        {
            if (!strcmp(task_list[index].Name,Name))
            {
                task_list[index].Duration = duration;
                if (task_list[index].Duration > task_list[index].Duration_Max)
                {   // determine max. duration
                    task_list[index].Duration_Max = task_list[index].Duration;
                } // if
                found = true;
            } // if
            index++;
        } // while
    } // if
    else return ERR_EMPTY;
    if (!found)
         return ERR_NAME;
    else return NO_ERR;
} // Scheduler::updateDuration()

/*-----------------------------------------------------------------------------
  Purpose  : Disable a task.
  Variables: Name: Name of task to disable
  Returns  : error [NO_ERR, ERR_NAME, ERR_EMPTY]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::disable_task(char *Name)
{
	uint8_t index = 0;
    uint8_t found = false;
	
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Status &= ~TASK_ENABLED;
                found = true;
			} // if
			index++;
		} // while
	} // if
	else return ERR_EMPTY;
	if (!found)
	     return ERR_NAME;
	else return NO_ERR;	
} // Scheduler::disable_task()

/*-----------------------------------------------------------------------------
  Purpose  : Set the time-period (msec.) of a task.
  Variables: msec: the time in milliseconds
             name: the name of the task to set the time for
  Returns  : error [NO_ERR, ERR_NAME, ERR_EMPTY]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::set_task_time_period(uint16_t Period, const char *Name)
{
	uint8_t index = 0;
    uint8_t found = false;
	
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Period = (uint16_t)(Period * TICKS_PER_SEC / 1000);
                found = true;
			} // if
			index++;
		} // while
	} // if
	else return ERR_EMPTY;
	if (!found)
	     return ERR_NAME;
	else return NO_ERR;	
} // Scheduler::set_task_time_period()

/*-----------------------------------------------------------------------------
  Purpose  : list all tasks and result
  Variables: -
  Returns  : fd: QString
  ---------------------------------------------------------------------------*/
QString Scheduler::list_all_tasks(void)
{
    uint8_t index = 0;
    QString fd;

    fd  = "Task-Name      T(ms) Stat T(μs) M(μs)\n";
    fd += "-------------------------------------\n";
    //go through the active tasks
    if(task_list[index].Period != 0)
    {
        while (task_list[index].Period != 0)
        {
            fd += QString("%1 %2 0x%3 %4 %5\n").arg(task_list[index].Name,14)
                                               .arg(task_list[index].Period * 1000 / TICKS_PER_SEC,5)
                                               .arg(task_list[index].Status,2,16,QLatin1Char('0'))
                                               .arg(task_list[index].Duration,5)
                                               .arg(task_list[index].Duration_Max,5);
            index++;
        } // while
    } // if
    return fd;
} // Scheduler::list_all_tasks()
