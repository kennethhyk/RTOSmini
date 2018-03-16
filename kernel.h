/* pointer to void f(void) */
typedef void (*voidfuncptr) (void);

/**
  * This internal kernel function is the context switching mechanism.
  * It is done in a "funny" way in that it consists two halves: the top half
  * is called "Exit_Kernel()", and the bottom half is called "Enter_Kernel()".
  * When kernel calls this function, it starts the top half (i.e., exit). Right in
  * the middle, "Cp" is activated; as a result, Cp is running and the kernel is
  * suspended in the middle of this function. When Cp makes a system call,
  * it enters the kernel via the Enter_Kernel() software interrupt into
  * the middle of this function, where the kernel was suspended.
  * After executing the bottom half, the context of Cp is saved and the context
  * of the kernel is restore. Hence, when this function returns, kernel is active
  * again, but Cp is not running any more.
  * (See file "switch.S" for details.)
  */
extern void CSwitch();
extern void Exit_Kernel();    /* this is the same as CSwitch() */

/**
  * This external function could be implemented in two ways:
  *  1) as an external function call, which is called by Kernel API call stubs;
  *  2) as an inline macro which maps the call into a "software interrupt";
  *       as for the AVR processor, we could use the external interrupt feature,
  *       i.e., INT0 pin.
  *  Note: Interrupts are assumed to be disabled upon calling Enter_Kernel().
  *     This is the case if it is implemented by software interrupt. However,
  *     as an external function call, it must be done explicitly. When Enter_Kernel()
  *     returns, then interrupts will be re-enabled by Enter_Kernel().
  */
extern void Enter_Kernel();

//========================
//  RTOS Internal      
//========================

/**
  *  This is the set of all possible priority levels for a task
  */
typedef enum priority_levels
{
	SYSTEM = 0,
	PERIODIC,
	RR,
  IDLE
} PRIORITY_LEVEL;

/**
  *  This is the set of states that a task can be in at any given time.
  */
typedef enum process_states {
  DEAD = 10,
  READY,
  RUNNING,
  BLOCKED
} PROCESS_STATES;

/**
  * This is the set of kernel requests, i.e., a request code for each system call.
  */
typedef enum kernel_request_type {
  NONE = 20,
  CREATE,
  NEXT,
  TERMINATE,
  TIMER
} KERNEL_REQUEST_TYPE;

/**
  * Each task is represented by a process descriptor, which contains all
  * relevant information about this task. For convenience, we also store
  * the task's stack, i.e., its workspace, in here.
  */
typedef struct process_descriptor
{
  PID pid;
  int arg; // integer function argument
  unsigned char *sp; // stack pointer for process memory
  unsigned char workSpace[WORKSPACE];
  TICK period;
  TICK wcet;
  TICK start_time;
  TICK remaining_ticks;
  TICK next_start;
  PROCESS_STATES state;
  voidfuncptr code; // function to be executed as part of task
  KERNEL_REQUEST_TYPE request;
  struct process_descriptor * next; //  next item in q for process
  PRIORITY_LEVEL priority;
} PD;

typedef struct queue
{
  char name[10];
  unsigned short size;
  PD *head;
  PD *tail;
} task_queue;

/** task queues for each priority of tasks*/
task_queue SYSTEM_TASKS;
task_queue PERIODIC_TASKS;
task_queue RR_TASKS;

/** application defined main funciton */
extern void a_main();
/** timer interrupt init function */
void init_timer();
/** Function to kill task */
void Task_Terminate();

void init_queue(task_queue * q); 
void enqueue(task_queue * q, PD * task);
PD * deque(task_queue * q);
PD * peek(task_queue * q);
void enqueue_in_offset_order(task_queue * q, PD * task);
void Setup_Function_Stack(PD *p, PID pid, voidfuncptr f);

/*   
* inline assembly code to disable/enable maskable interrupts   
* (N.B. Use with caution.)  
*/  

#define OS_EI()    asm(" sei ")  /* disable all interrupts */
#define OS_DI()    asm(" cli ")  /* enable all interrupts */
