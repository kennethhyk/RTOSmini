#include <string.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#include "kernel.h"
#include "os.h"
#include "queue.c"

// #define DEBUG

/**
  *  Create a new task
*/
PD *static void Kernel_Create_Task(voidfuncptr f, int arg, PRIORITY_LEVEL level)
{
  int x;
  PD *p = NULL;

  if (TotalTasks == MAXTHREAD)
  {
    return p; // Too many tasks!
  }

  // find a DEAD PD that we can use
  for (x = 0; x < MAXTHREAD; x++)
  {
    if (Process[x].state == DEAD)
    {
      p = &(Process[x]);
      break;
    }
  }

  TotalTasks++;
  Setup_Function_Stack(p, x, f);

  p->priority = level;
  p->arg = arg;
  return p;
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
  p->ticks_remaining = 0;
}

// Creates system task and enqueues into SYSTEM_TASKS queue
PID Task_Create_System(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = SYSTEM;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
  {
    return -1; // Too many tasks :(
  }

  enqueue(&SYSTEM_TASKS, p);
  return p->pid;
}

// Creates RR task and enqueues into RR_TASKS queue
PID Task_Create_RR(voidfuncptr f, int arg)
{
  enum PRIORITY_LEVEL priority = RR;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
  {
    return -1; // Too many tasks :(
  }

  enqueue(&RR_TASKS, p);
  return p->pid;
}

// Creates periodic task and enqueues into
// PERIODIC_TASKS queue in order of start time
PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset)
{
  enum PRIORITY_LEVEL priority = PERIODIC;
  PD *p = Kernel_Create_Task(f, arg, priority);
  if (p == NULL)
  {
    return -1; // Too many tasks :(
  }

  enqueue_in_offset_order(&PERIODIC_TASKS, p);

  // set periodic task specific attributes
  p->period = period;
  p->wcet = wcet;
  p->start_time = num_ticks + offset;
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
  if (Cp->state == RUNNING)
  {
    return;
  }

  // Look through q's and pick task to run according to q precedence
  if (SYSTEM_TASKS.head && peek(&SYSTEM_TASKS)->state != BLOCKED)
  {
    Cp = peek(&SYSTEM_TASKS);
  }
  // periodic tasks are sorted by start time, so only looking at head suffices
  else if (PERIODIC_TASKS.head && num_ticks >= peek(&PERIODIC_TASKS)->start_time)
  {
    Cp = peek(&PERIODIC_TASKS);
  }
  else if (RR_TASKS.size > 0)
  {
    // go through the q and find
    while (peek(&RR_TASKS)->state == BLOCKED)
    {
      // idle task exists in rrq so this loop WILL terminate
      enqueue(&RR_TASKS, deque(&RR_TASKS));
    }
    Cp = peek(&RR_TASKS);
  }

  CurrentSp = Cp->sp;
  Cp->state = RUNNING;
}

/**
  * This internal kernel function is the "main" driving loop of this full-served
  * model architecture. On OS_Start(), the kernel repeatedly
  * requests the next available user task's execution, and then invokes 
  * the corresponding kernel function on its behalf.
  *
  */
static void Next_Kernel_Request()
{
  Dispatch(); /* select a new task to run */

  while (1)
  {
    Cp->request = NONE; /* clear its request */

    // activate this newly selected task
    CurrentSp = Cp->sp;
    // the context switching code now replaces
    // physical stack pointer with this value
    Exit_Kernel();

    // program counter returns here on
    // a call to Task_Terminate after
    // dispatched function returns

    /* save the Cp's stack pointer */
    Cp->sp = CurrentSp;

    switch (Cp->request)
    {
    case TIMER:
      // Tasks gets interrupted
      // Reaches here from ISR
      switch (Cp->type)
      {
      case SYSTEM:
        // nothing to do, pass
        break;
      case PERIODIC:
        // reduce ticks
        Cp->ticks_remaining--;
        if (Cp->ticks_remaining <= 0)
        {
          // not good
          OS_Abort(-1);
        }
        break;
      case RR:
        Cp->ticks_remaining--;
        if (Cp->ticks_remaining <= 0)
        {
          // reset ticks and move to back of q
          Cp->ticks_remaining = 1;
          enqueue(&RR_TASKS, deque(&RR_TASKS));
        }
        break;
      }
      if (Cp->state != BLOCKED)
        Cp->state = READY;
      Dispatch();
      break;

    case NEXT:
      // Tasks giving away control voluntarily (i.e. yield)
      switch (Cp->type)
      {
      case SYSTEM:
        // dequeue and enqueue
        enqueue(&SYSTEM_TASKS, deque(&SYSTEM_TASKS));
        break;

      case PERIODIC:
        // dequeue, reset start time, ticks_remaining
        // and enqueue in order in q
        deque(&PERIODIC_TASKS);
        Cp->start_time = Cp->start_time + Cp->period;
        Cp->ticks_remaining = Cp->wcet;
        enqueue_in_offset_order(&PERIODIC_TASKS, Cp);
        break;

      case RR:
        // RR task yielding
        // reset ticks and move to back of q
        Cp->ticks_remaining = 1;
        enqueue(&RR_TASKS, deque(&RR_TASKS));
        break;
      }
      // change state of current process and dispatch
      if (Cp->state != BLOCKED){
        Cp->state = READY;
      }
      // choose new task to run
      Dispatch();
      break;

    case NONE:
      /* NONE could be caused by a timer interrupt */
      if (Cp->state != BLOCKED){
        Cp->state = READY;
      }
      Dispatch();
      break;

    case TERMINATE:
      /* deallocate all resources used by this task */
      switch (Cp->type)
      {
      case SYSTEM:
        deque(&SYSTEM_TASKS);
        break;
      case PERIODIC:
        deque(&PERIODIC_TASKS);
        break;
      case RR:
        deque(&RR_TASKS);
        break;
      }

      Cp->state = DEAD;
      Dispatch();
      break;

    default:
      /* Houston! we have a problem here! */
      break;
    }
  }
}

/*========================
  |  RTOS API and Stubs  |
  *=======================
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
  //Clear memory for each thread/process stack
  for (x = 0; x < MAXTHREAD; x++)
  {
    memset(&(Process[x]), 0, sizeof(PD));
    Process[x].state = DEAD;
  }

  queue_init(&system_tasks);
  queue_init(&periodic_tasks);
  queue_init(&rr_tasks);
}

/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start()
{
  OS_DI();
  if ((!KernelActive) && (TotalTasks > 0))
  {
    init_timer();
    // Select a free task and dispatch it
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

void idle_func()
{
  while (1)
  {
    ;
  }
}

void init_timer()
{
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15999;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS00);
  TIMSK1 |= (1 << OCIE1A);
  OS_EI();
}

ISR(TIMER1_COMPA_vect)
{
  num_ticks++;
  OS_DI();
  Cp->state = TIMER;
  Enter_Kernel();
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
  // DDRB = 0b11000000;
  OS_Start();
}
