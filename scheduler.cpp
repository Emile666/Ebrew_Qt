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

#include "scheduler.h"

Scheduler::Scheduler(void) : QObject()
{
	memset(task_list,0x00,sizeof(task_list)); // clear task_list array
} // Scheduler::Scheduler()

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

	while(task_list[index].pFunction)
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
        LARGE_INTEGER lInt1,lInt2;

	//go through the active tasks
	while(task_list[index].pFunction)
	{
		if((task_list[index].Status & (TASK_READY | TASK_ENABLED)) == (TASK_READY | TASK_ENABLED))
		{
                   QueryPerformanceCounter(&lInt1);
		   task_list[index].pFunction(); // run the task
		   task_list[index].Status &= ~TASK_READY; // reset the task when finished
		   task_list[index].Counter = task_list[index].Period; // reset counter
                   QueryPerformanceCounter(&lInt2);
                   task_list[index].Duration = (uint16_t)(lInt2.QuadPart - lInt1.QuadPart);
                   if (task_list[index].Duration > task_list[index].Duration_Max)
		   {
		      task_list[index].Duration_Max = task_list[index].Duration;
                   } // if
		} // if
		index++;
	} // while
	//go to sleep till next tick!
} // Scheduler::dispatch_tasks()

/*-----------------------------------------------------------------------------
  Purpose  : Add a function to the task-list struct. Should be called upon
  		     initialization.
  Variables: task_ptr: pointer to function
             delay   : initial delay in msec.
             period  : period between two calls in msec.
  Returns  : [NO_ERR, ERR_MAX_TASKS]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::add_task(void (*task_ptr), char *Name, uint16_t delay, uint16_t period)
{
	uint8_t  index = 0;
	uint16_t temp1 = (uint16_t)(delay  * TICKS_PER_SEC / 1000);
	uint16_t temp2 = (uint16_t)(period * TICKS_PER_SEC / 1000);

	if (max_tasks >= MAX_TASKS)
		return ERR_MAX_TASKS;
		
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while(task_list[++index].Period != 0) ;
	} // if

	if(task_list[index].Period == 0)
	{
		task_list[index].pFunction = task_ptr;     // Pointer to Function
		task_list[index].Period    = temp2;        // Period in msec.
		task_list[index].Counter   = temp2;	       // Countdown timer
		task_list[index].Delay     = temp1;        // Initial delay before start
		task_list[index].Status   |= TASK_ENABLED; // Eable task by default
		task_list[index].Status   &= ~TASK_READY;  // Task not ready to run
		task_list[index].Duration     = 0;              // Actual Task Duration
		task_list[index].Duration_Max = 0;              // Max. Task Duration
		strncpy(task_list[index].Name, Name, NAME_LEN); // Name of Task
		max_tasks++; // increase number of tasks
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
	uint8_t found = FALSE;

	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Status |= TASK_ENABLED;
				found = TRUE;
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
  Purpose  : Disable a task.
  Variables: Name: Name of task to disable
  Returns  : error [NO_ERR, ERR_NAME, ERR_EMPTY]
  ---------------------------------------------------------------------------*/
uint8_t Scheduler::disable_task(char *Name)
{
	uint8_t index = 0;
	uint8_t found = FALSE;
	
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Status &= ~TASK_ENABLED;
				found = TRUE;
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
uint8_t set_task_time_period(uint16_t Period, char *Name)
{
	uint8_t index = 0;
	uint8_t found = FALSE;
	
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
		while ((task_list[index].Period != 0) && !found)
		{
			if (!strcmp(task_list[index].Name,Name))
			{
				task_list[index].Period = (uint16_t)(Period * TICKS_PER_SEC / 1000);
				found = TRUE;
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
  Purpose  : list all tasks and send result to USB-RS232
  Variables: fd: FILE pointer to an opened file-descriptor
  Returns  : -
  ---------------------------------------------------------------------------*/
void Scheduler::list_all_tasks(FILE *fd)
{
	uint8_t index = 0;
        LARGE_INTEGER li;
        double  d;

        QueryPerformanceFrequency(&li);
        d = 1000.0 / li.QuadPart; // convert to msec.
	fprintf(fd,"Task-Name      T(ms) Stat T(ms) M(ms)\n");
        fprintf(fd,"-------------------------------------\n");
	//go through the active tasks
	if(task_list[index].Period != 0)
	{
            while (task_list[index].Period != 0)
	    {
	    	fprintf(fd,"%-14s %5d 0x%02x %5.1f %5.1f\n",
                           task_list[index].Name,
	    	           task_list[index].Period * 1000 / TICKS_PER_SEC,
                           task_list[index].Status,
                           d * task_list[index].Duration,
                           d * task_list[index].Duration_Max);
	    	index++;
	    } // while
	} // if
} // Scheduler::list_all_tasks()
