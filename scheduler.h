/*==================================================================
  File Name    : scheduler.h
  Author       : Emile
  ------------------------------------------------------------------
  Purpose : This is the header-file for scheduler.c
  ==================================================================
*/ 
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

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
	void     (* pFunction)(void); // Function pointer
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

public:
	Scheduler(void);
	void    scheduler_isr(void);  // run-time function for scheduler
	void    dispatch_tasks(void); // run all tasks that are ready
	uint8_t add_task(void (*task_ptr), char *Name, uint16_t delay, uint16_t period);
	uint8_t set_task_time_period(uint16_t Period, char *Name);
	uint8_t enable_task(char *Name);
	uint8_t disable_task(char *Name);
	void    list_all_tasks(FILE *fd);

protected:
	task_struct task_list[MAX_TASKS]; // struct with all tasks
	uint8_t     max_tasks = 0;        // actual number of tasks added

} // class Scheduler

#endif
