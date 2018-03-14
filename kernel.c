#include <string.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#include "kernel.h"
#include "os.h"
#include "queue.c"

/**
 * \file active.c
 * \brief A Skeleton Implementation of an RTOS
 * 
 * \mainpage A Skeleton Implementation of a "Full-Served" RTOS Model
 * This is an example of how to implement context-switching based on a 
 * full-served model. That is, the RTOS is implemented by an independent
 * "kernel" task, which has its own stack and calls the appropriate kernel 
 * function on behalf of the user task.
 *
 * \author Dr. Mantis Cheng
 * \date 29 September 2006
 *
 * ChangeLog: Modified by Alexander M. Hoole, October 2006.
 *			  -Rectified errors and enabled context switching.
 *			  -LED Testing code added for development (remove later).
 *
 * \section Implementation Note
 * This example uses the ATMEL AT90USB1287 instruction set as an example
 * for implementing the context switching mechanism. 
 * This code is ready to be loaded onto an AT90USBKey.  Once loaded the 
 * RTOS scheduling code will alternate lighting of the GREEN LED light on
 * LED D2 and D5 whenever the correspoing PING and PONG tasks are running.
 * (See the file "cswitch.S" for details.)
 */

//Comment out the following line to remove debugging code from compiled version.
// #define DEBUG

void timer1_init()
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15999;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS00);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  if (KernelActive)
  {
    OS_DI();
    Cp->state = READY;
    Enter_Kernel();
    Next_Kernel_Request();
    OS_EI();
  }
}

/**
  *  Create a new task
*/
PD *static void Kernel_Create_Task(voidfuncptr f, int arg, PRIORITY_LEVEL level)
{
  int x;
  PD *p = NULL;

  if (TotalTasks == MAXTHREAD)
    return p; // Too many tasks!

  // find a DEAD PD that we can use
  for (x = 0; x < MAXTHREAD; x++)
  {
    if (Process[x].state == DEAD)
    {
      p = &(Process[x]);
    }
  }

  TotalTasks++;
  Setup_Function_Stack(p, f);

  p->priority = level;
  p->arg = arg
  return p;
}

/**
 * Setup function stack and PD
 */
void Setup_Function_Stack(PD *p, voidfuncptr f)
{
  unsigned char *sp;

  //Changed -2 to -1 to fix off by one error.
  sp = (unsigned char *)&(p->workSpace[WORKSPACE - 1]);

  //Initialize the workspace (i.e., stack) and PD here!

  //Clear workspace
  memset(&(p->workSpace), 0, WORKSPACE);

  //Notice that we are placing the address (16-bit) of the functions
  //onto the stack in reverse byte order (least significant first, followed
  //by most significant).  This is because the "return" assembly instructions
  //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
  //second), even though the AT90 is LITTLE ENDIAN machine.

  //Store terminate at the bottom of stack to protect against stack underrun.
  *(unsigned char *)sp-- = ((unsigned int)Task_Terminate) & 0xff;
  *(unsigned char *)sp-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;
  *(unsigned char *)sp-- = 0x00;

  //Place return address of function at bottom of stack
  *(unsigned char *)sp-- = ((unsigned int)f) & 0xff;
  *(unsigned char *)sp-- = (((unsigned int)f) >> 8) & 0xff;
  *(unsigned char *)sp-- = 0x00;

  //Place stack pointer at top of stack
  sp = sp - 34;

  p->pid = x;
  p->sp = sp;  /* stack pointer into the "workSpace" */
  p->code = f; /* function to be executed as a task */
  p->request = NONE;
  p->state = READY;
  p->next = NULL;
}

PID Task_Create_System(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = SYSTEM;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL) return -1; // Too many tasks :(

  enqueue(&SYSTEM_TASKS, p);
  return p->pid;
}

PID Task_Create_RR(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = RR;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL) return -1; // Too many tasks :(

  enqueue(&RR_TASKS, p);
  return p->pid;
}

PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset)
{
  enum PRIORITY_LEVEL priority = PERIODIC;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL) return -1; // Too many tasks :(

  enqueue(&PERIODIC_TASKS, p);
  
  // set periodic task specific attributes 
  p->period = period;
  p->wcet = wcet;
  p->offset = offset;
  return p->pid;
}

int  Task_GetArg(void){
  return Cp->arg;
}

PID  Task_Pid(void){
  return Cp->pid;
}

/**
  * This internal kernel function is a part of the "scheduler". It chooses the 
  * next task to run, i.e., Cp.
  */
static void Dispatch()
{
  /* Find the next READY task
  *  Note: if there is no READY task, then this will loop forever!.
  */

  while (Process[NextP].state != READY)
  {
    NextP = (NextP + 1) % MAXTHREAD;
  }

  Cp = &(Process[NextP]);
  CurrentSp = Cp->sp;
  Cp->state = RUNNING;

  NextP = (NextP + 1) % MAXTHREAD;
}

/**
  * This internal kernel function is the "main" driving loop of this full-served
  * model architecture. Basically, on OS_Start(), the kernel repeatedly
  * requests the next user task's next system call and then invokes the
  * corresponding kernel function on its behalf.
  *
  * This is the main loop of our kernel, called by OS_Start().
  */
static void Next_Kernel_Request()
{
  Dispatch(); /* select a new task to run */

  while (1)
  {
    Cp->request = NONE; /* clear its request */

    /* activate this newly selected task */
    CurrentSp = Cp->sp;
    Exit_Kernel(); /* or CSwitch() */

    /* if this task makes a system call, it will return to here! */

    /* save the Cp's stack pointer */
    Cp->sp = CurrentSp;

    switch (Cp->request)
    {
    case CREATE:
      Kernel_Create_Task(Cp->code);
      break;
    case NEXT:
    case NONE:
      /* NONE could be caused by a timer interrupt */
      Cp->state = READY;
      Dispatch();
      break;
    case TERMINATE:
      /* deallocate all resources used by this task */
      Cp->state = DEAD;
      Dispatch();
      break;
    default:
      /* Houston! we have a problem here! */
      break;
    }
  }
}

/*================
  * RTOS  API  and Stubs
  *================
  */

/**
  * This function initializes the RTOS and must be called before any other
  * system calls.
  */
void OS_Init()
{
  int x;

  TotalTasks = 0;
  KernelActive = 0;
  NextP = 0;
  //Reminder: Clear the memory for the task on creation.
  for (x = 0; x < MAXTHREAD; x++)
  {
    memset(&(Process[x]), 0, sizeof(PD));
    Process[x].state = DEAD;
  }
}

/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start()
{
  if ((!KernelActive) && (TotalTasks > 0))
  {
    OS_DI();
    /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

    /* here we go...  */
    KernelActive = 1;
    Next_Kernel_Request();
    /* NEVER RETURNS!!! */
  }
}

/**
  * For this example, we only support cooperatively multitasking, i.e.,
  * each task gives up its share of the processor voluntarily by calling
  * Task_Next().
  */
void Task_Create(voidfuncptr f)
{
  if (KernelActive)
  {
    OS_DI();
    Cp->request = CREATE;
    Cp->code = f;
    Enter_Kernel();
  }
  else
  {
    /* call the RTOS function directly */
    Kernel_Create_Task(f);
  }
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
  if (KernelActive)
  {
    OS_DI();
    Cp->request = NEXT;
    Enter_Kernel();
  }
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate()
{
  if (KernelActive)
  {
    OS_DI();
    Cp->request = TERMINATE;
    Enter_Kernel();
    /* never returns here! */
  }
}

/*============
  * A Simple Test 
  *============
  */

/**
  * A cooperative "Ping" task.
  * Added testing code for LEDs.
  */
void Ping()
{
  for (;;)
  {
    // PORTB = 0b10000000;
    // _delay_ms(100);
    // Task_Next();
    PORTB = 0b00000000;
  }
}

/**
  * A cooperative "Pong" task.
  * Added testing code for LEDs.
  */
void Pong()
{
  for (;;)
  {
    // PORTB = 0b00000000;
    // _delay_ms(100);
    // Task_Next();
    PORTB = 0b10000000;
  }
}

/**
  * This function creates two cooperative tasks, "Ping" and "Pong". Both
  * will run forever.
  */
void main()
{
  OS_Init();
  Task_Create(Pong);
  Task_Create(Ping);
  DDRB = 0b11000000;
  timer1_init();
  OS_Start();
}
