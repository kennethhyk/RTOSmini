#include <string.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#include "kernel.h"
#include "os.h"
#include "queue.c"

// #define DEBUG

void idle_func(){
  while(1);
}

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
  num_ticks++;
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
  Setup_Function_Stack(p, pid, f);

  p->priority = level;
  p->arg = arg return p;
}

/**
 * Setup function stack and PD
 */
void Setup_Function_Stack(PD *p, PID pid, voidfuncptr f)
{
  unsigned char *sp;

  sp = (unsigned char *)&(p->workSpace[WORKSPACE - 1]);

  //Clear workspace
  memset(&(p->workSpace), 0, WORKSPACE);

  //Notice that we are placing the address (17-bit) of the functions
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

  p->pid = pid;
  p->sp = sp;
  p->code = f;
  p->request = NONE;
  p->state = READY;
  p->next = NULL;
}

// Creates system task and enqueues into SYSTEM_TASKS queue
PID Task_Create_System(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = SYSTEM;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
    return -1; // Too many tasks :(

  enqueue(&SYSTEM_TASKS, p);
  return p->pid;
}

// Creates RR task and enqueues into RR_TASKS queue
PID Task_Create_RR(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = RR;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
    return -1; // Too many tasks :(

  enqueue(&RR_TASKS, p);
  return p->pid;
}

// Creates periodic task and enqueues into PERIODIC_TASKS queue
PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset)
{
  enum PRIORITY_LEVEL priority = PERIODIC;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
    return -1; // Too many tasks :(

  enqueue(&PERIODIC_TASKS, p);

  // set periodic task specific attributes
  p->period = period;
  p->wcet = wcet;
  p->next_start = num_ticks + offset;
  p->ticks_remaining = wcet;

  return p->pid;
}

int Task_GetArg(void)
{
  return Cp->arg;
}

PID Task_Pid(void)
{
  return Cp->pid;
}

/**
  * This internal kernel function is a part of the "scheduler". It chooses the 
  * next task to run, i.e., Cp.
  */
static void Dispatch()
{
  if (Cp->state == RUNNING) return;

  // Look through q's and pick task to run according to precedence
  if (SYSTEM_TASKS.head && peek(&SYSTEM_TASKS)->state != BLOCKED)
  {
    Cp = peek(&SYSTEM_TASKS);
  }
  // periodic tasks are sorted by start time
  else if (PERIODIC_TASKS.head > 0 && num_ticks >= peek(&PERIODIC_TASKS)->next_start)
  {
    Cp = peek(&PERIODIC_TASKS);
  }
  else if (RR_TASKS.size > 0) 
  {
    while (peek(&RR_TASKS)->state == BLOCKED)
    {
      enqueue(&RR_TASKS, deque(&RR_TASKS));
    }
    // idle task exists in rrq
    Cp = peek(&rr_tasks);
  }

  CurrentSp = Cp->sp;
  Cp->state = RUNNING;
}

/**
  * This internal kernel function is the "main" driving loop of this full-served
  * model architecture. On OS_Start(), the kernel repeatedly
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
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
  OS_DI();
  Cp->state = READY;
  Cp->request = NEXT;
  Enter_Kernel();
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate()
{
  OS_DI();
  Cp->request = TERMINATE;
  Cp->state = DEAD;
  // Process[Cp->pid].state = DEAD;
  Tasks--;
  Enter_Kernel();
  /* never returns here! */
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
