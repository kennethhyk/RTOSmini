#include "os.h"

// pointer to void f(void) 
typedef void (*voidfuncptr)(void); 

//========================
//  RTOS Internal      
//========================

/**
  *  This is the set of states that a task can be in at any given time.
  */
typedef enum process_states {
  DEAD = 0,
  READY,
  RUNNING
} PROCESS_STATES;

/**
  * This is the set of kernel requests, i.e., a request code for each system call.
  */
typedef enum kernel_request_type {
  NONE = 0,
  CREATE,
  NEXT,
  TERMINATE
} KERNEL_REQUEST_TYPE;

/**
  * Each task is represented by a process descriptor, which contains all
  * relevant information about this task. For convenience, we also store
  * the task's stack, i.e., its workspace, in here.
  */
typedef struct process_descriptor
{
  unsigned char *sp; // stack pointer for process memory
  unsigned char workSpace[WORKSPACE];
  PROCESS_STATES state;
  voidfuncptr code; // function to be executed as part of task
  KERNEL_REQUEST_TYPE request;
  PD * queue_next; //  next item in q for process
} PD;

typedef struct queue
{
  unsigned short size;
  PD *head;
  PD *tail;
} task_queue;

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXTHREAD];

/**
  * The process descriptor of the currently RUNNING task.
  */
volatile static PD *Cp;

/** 
  * Since this is a "full-served" model, the kernel is executing using its own
  * stack. We can allocate a new workspace for this kernel stack, or we can
  * use the stack of the "main()" function, i.e., the initial C runtime stack.
  * (Note: This and the following stack pointers are used primarily by the
  *   context switching code, i.e., CSwitch(), which is written in assembly
  *   language.)
  */
volatile unsigned char *KernelSp;

/**
  * This is a "shadow" copy of the stack pointer of "Cp", the currently
  * running task. During context switching, we need to save and restore
  * it into the appropriate process descriptor.
  */
volatile unsigned char *CurrentSp;

/** index to next task to run */
volatile static unsigned int NextP;

/** 1 if kernel has been started; 0 otherwise. */
volatile static unsigned int KernelActive;

/** number of tasks created so far */
volatile static unsigned int TotalTasks;

/** task queues for each priority of tasks*/
task_queue SYSTEM_TASKS;
task_queue PERIODIC_TASKS;
task_queue ROUND_ROBIN_TASKS;

/** application defined main funciton */
extern void a_main();