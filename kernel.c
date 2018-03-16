#include <stdio.h>
#include <stdbool.h>
#include "os.h"
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "./LED/LED_Test.c"
#include "kernel.h"
#include "queue.c"
#include "avr_console.h"

#define DEBUG 1

void Ping();
void Pong();

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXTHREAD];

/**
  * The process descriptor of the running task
  */
static volatile PD *Cp;

/** 
  * Since this is a "full-served" model, the kernel is executing using its own
  * stack. We can allocate a new workspace for this kernel stack, or we can
  * use the stack of the "main()" function, i.e., the initial C runtime stack.
  * (Note: This and the following stack pointers are used primarily by the
  *   context switching code, i.e., CSwitch(), which is written in assembly
  *   language.)
*/

volatile unsigned char *KernelSp;

/** 1 if kernel has been started; 0 otherwise. */
static volatile unsigned int KernelActive;

/** number of tasks created so far */
static volatile unsigned int TotalTasks;

// Tick count in order to schedule periodic tasks
volatile unsigned int num_ticks;

/**
  * This is a "shadow" copy of the stack pointer of "Cp", the currently
  * running task. During context switching, we need to save and restore
  * it into the appropriate process descriptor.
*/
unsigned char *CurrentSp;

/**
  *  Create a new task
*/
static PD * Kernel_Create_Task(voidfuncptr f, int arg, PRIORITY_LEVEL level)
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
  p->remaining_ticks = 0;
}

// Creates system task and enqueues into SYSTEM_TASKS queue
PID Task_Create_System(voidfuncptr f, int arg)
{
  PD *p = Kernel_Create_Task(f, arg, SYSTEM);
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
  printf("Creating rr tasks;"); 
  PD *p = Kernel_Create_Task(f, arg, RR);
  if (p == NULL)
  {
    return -1; // Too many tasks :(
  }

  p->remaining_ticks = 1;
  enqueue(&RR_TASKS, p);
  printf("size of rrq: %d\n", RR_TASKS.size);
  return p->pid;
}

// Creates periodic task and enqueues into
// PERIODIC_TASKS queue in order of start time
PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset)
{
  PD *p = Kernel_Create_Task(f, arg, PERIODIC);
  if (p == NULL)
  {
    return -1; // Too many tasks :(
  }

  enqueue_in_start_order(&PERIODIC_TASKS, p);

  // set periodic task specific attributes
  p->period = period;
  p->wcet = wcet;
  p->start_time = num_ticks + offset;
  p->next_start = p->start_time + period;
  p->remaining_ticks = wcet;
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
  if ((SYSTEM_TASKS.size > 0) && (peek(&SYSTEM_TASKS)->state != BLOCKED))
  {
    Cp = peek(&SYSTEM_TASKS);
    // toggle_LED_B3();
  }
  // periodic tasks are sorted by start time, so only looking at head suffices
  else if (PERIODIC_TASKS.head && num_ticks >= peek(&PERIODIC_TASKS)->start_time)
  {
    Cp = peek(&PERIODIC_TASKS);
  }
  else if (RR_TASKS.size > 0)
  // else
  {
    // go through the q and find
    while (peek(&RR_TASKS)->state == BLOCKED)
    {
      // idle task exists in rrq so this loop WILL terminate
      enqueue(&RR_TASKS, deque(&RR_TASKS));
    }
    Cp = peek(&RR_TASKS);
  }
  else {
    printf("HOUSTON, WE HAVE A PROBLEM!");
    OS_Abort(1);
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
      switch (Cp->priority)
      {
      case SYSTEM:
        // nothing to do, pass
        break;
      case PERIODIC:
        // reduce ticks
        Cp->remaining_ticks--;
        if (Cp->remaining_ticks <= 0)
        {
          // not good
          OS_Abort(-1);
        }
        break;
      case RR:
        Cp->remaining_ticks--;
        if (Cp->remaining_ticks <= 0)
        {
          // reset ticks and move to back of q
          Cp->remaining_ticks = 1;
          enqueue(&RR_TASKS, deque(&RR_TASKS));
         }
        break;
      }

      if (Cp->state != BLOCKED){
        Cp->state = READY;
      }

      Dispatch();
      break;

    case NEXT:
      // Tasks giving away control voluntarily (i.e. yield)
      switch (Cp->priority)
      {
      case SYSTEM:
        // dequeue and enqueue
        enqueue(&SYSTEM_TASKS, deque(&SYSTEM_TASKS));
        break;

      case PERIODIC:
        // dequeue, reset start time, remaining_ticks
        // and enqueue in order in q
        deque(&PERIODIC_TASKS);
        Cp->start_time = Cp->start_time + Cp->period;
        Cp->remaining_ticks = Cp->wcet;
        enqueue_in_offset_order(&PERIODIC_TASKS, Cp);
        break;

      case RR:
        // RR task yielding
        // reset ticks and move to back of q
        Cp->remaining_ticks = 1;
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
      switch (Cp->priority)
      {
      case SYSTEM:
        deque(&SYSTEM_TASKS);
        break;
      case PERIODIC:
        // periodic tasks run forever
        // so reset in q
        deque(&PERIODIC_TASKS);
        Cp->start_time = Cp->next_start;
        Cp->next_start += Cp->period;
        Cp->state = READY;
        Cp->request = NONE;
        enqueue_in_start_order(&PERIODIC_TASKS, Cp);
        break;
      case RR:
        deque(&RR_TASKS);
        // toggle_LED_B6();
        break;
      }

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

  init_queue(&SYSTEM_TASKS);
  init_queue(&PERIODIC_TASKS);
  init_queue(&RR_TASKS);
}

/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start()
{
  OS_DI();
  if ((KernelActive == 0) && (TotalTasks > 0))
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
  // memset(Cp, 0, sizeof(PD));
  Cp->request = TERMINATE;
  Cp->state = DEAD;
  TotalTasks--;
  Enter_Kernel();
  /* never returns here! */
}

void idle_func()
{
  while(1){
    printf("idle\n");
    toggle_LED_idle();
    _delay_ms(1000);
  }
}

void init_timer()
{
  //Clear timer config.
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B

  // OCR1A = 15999;       // for 10ms
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // TCCR1B |= (1 << CS00); // for 10ms
  // TIMSK1 |= (1 << OCIE1A); // for 10ms

// for 1s
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
// for 1s

  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  // OS_EI();
}

ISR(TIMER1_COMPA_vect)
{
  toggle_LED_B3();
  num_ticks++;
  OS_DI();
  Cp->request = TIMER;
  Enter_Kernel();
}


void OS_Abort(unsigned int error)
{
  OS_DI();
  while(1){
    // toggle_LED_B3();
    _delay_ms(500);
  }
}

void Pong()
{ 
  printf("started pong\n");
  toggle_LED_B6();
  _delay_ms(4000);
  toggle_LED_B6();
  printf("finished pong\n");
  // Task_Create_System(Ping, 0);
}

void Ping()
{
  printf("Started Ping\n");
  toggle_LED_B5();
  _delay_ms(2000);
  toggle_LED_B5();
  // Task_Create_RR(Pong, 0);
  printf("Finished Ping");
}

/**
  * This function creates two cooperative tasks, "Ping" and "Pong". Both
  * will run forever.
  */
void main()
{

  uart_init();
  stdout = &uart_output;
  stdin = &uart_input;

  init_LED_idle();
  init_LED_B5();
  init_LED_B6();
  init_LED_B3();

  // test lights
  // for(;;){
    // toggle_LED_B6();
    // _delay_ms(500);
    // toggle_LED_B5();
    // _delay_ms(500);
    // toggle_LED_B3();
    // _delay_ms(500);
    // toggle_LED_idle();
    // _delay_ms(500);
  // }

  printf("=========\n");
  // clear memory and prepare queues
  OS_Init();
  
  Task_Create_RR(idle_func, 0);
  // deque(&RR_TASKS);
  Task_Create_System(Ping, 0);
  // Task_Create_System(Pong, 0);
  // // Task_Create_RR(Pong, 0);
  // // Task_Create_RR(Pong, 0);
  OS_Start();

}
