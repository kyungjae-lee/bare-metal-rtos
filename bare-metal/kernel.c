/*******************************************************************************
 * File		: kernel.c
 * Brief	: Implementation of RTOS kernel
 * Author	: Kyungjae Lee
 * Date		: 05/04/2023
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "led.h"

/* Global variables */
uint8_t curr_task = 1;	/* Denotes the current task running on the CPU (Initialize to Task 1) */
uint32_t global_tick_count;

/* Structure for Task Control Blocks (TCBs) */
typedef struct
{
	uint32_t psp;					/* Task stack pointer */
	uint32_t block_count;			/* How long it should block */
	uint8_t state;					/* Task state */
	void (*task_handler)(void);		/* Function pointer to task handler */
} TCB_t;

/* Array to manage task control block handles */
TCB_t tcbs[NUM_TASKS];

/* 
 * Idle task handler()
 * Brief	: Idle task handler
 * Param	: None
 * Retval	: None
 * Note		: Idle task will run only when all other user tasks are in BLOCKED
 * 			  state.
 */
void idle_task_handler(void)
{
	while (1);
} /* End of idle_task_handler */

/* 
 * schedule()
 * Brief	: Triggers context switching by setting the PendSV exception
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void schedule(void)
{
	/* Pend the PendSV exception */
	ICSR |= PENDSVSET;
} /* End of schedule */

/* /
/* 
 * block_task()
 * Brief	: Blocks the task that calls this function for tick_count ticks 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void block_task(uint32_t tick_count)
{
	/* To prevent race condition on global variables, make sure to globally 
	   disable interrupt */
	DISABLE_INTERRUPTS();
		/* Now, until ENABLE_INTERRUPT, only Thread mode code will run. */

	/* Do not allow changing the idle task state to BLOCKED. */
	if (curr_task == 0)
		return;

	/* Store in the block_count the timestamp to unblock the task */
	tcbs[curr_task].block_count = global_tick_count + tick_count;

	/* Switch task state to BLOCKED */
	tcbs[curr_task].state = BLOCKED;

	/* Allow other task to run */
	schedule();

	/* Globally enable interrupt */
	ENABLE_INTERRUPTS();
} /* End of block_task */

/*
 * init_systick_timer()
 * Brief	: Initializes SysTick Timer
 * Param	: @tick_hz
 * Retval	: None
 * Note		: N/A
 */
void init_systick_timer(uint32_t tick_hz)
{
	uint32_t start_val = (SYSTICK_TIM_CLK / tick_hz) - 1;

	/* Clear the least significant 24 bits in the SYST_RVR */
	SYST_RVR &= ~0x00FFFFFF;

	/* Load the counter start value into SYST_RVR */
	SYST_RVR |= start_val;

	/* Configure SYST_CSR */
	SYST_CSR |= (TICKINT | CLKSOURCE | ENABLE);
		/* TICKINT	: Enable SysTick exception request */
		/* CLKSOURCE: Specify clock source; processor clock source */
		/* ENABLE	: Enable counter */
} /* End of init_systick_timer */

/* 
 * init_sched_stack()
 * Brief	: Initializes stack of scheduler
 * Param	: @sched_top_of_stack
 * Retval	: None
 * Note		: N/A
 */
__attribute__((naked)) void init_sched_stack(uint32_t sched_top_of_stack)
{
	//__asm volatile("msr msp, r0");
	__asm volatile("msr msp, %0": : "r" (sched_top_of_stack) : );
	__asm volatile("bx lr");	/* Return to the caller */
} /* End of init_sched_stack */

/* 
 * get_psp()
 * Brief	: Retrieves the current task's psp value from its TCB
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
uint32_t get_psp(void)
{
	return tcbs[curr_task].psp;
} /* End of get_psp */

/* 
 * save_psp()
 * Brief	: Saves the current task's psp value to its TCB
 * Param	: @curr_psp - current tasks's psp value
 * Retval	: None
 * Note		: N/A
 */
void save_psp(uint32_t curr_psp)
{
	tcbs[curr_task].psp = curr_psp;
} /* End of save_psp */

/* 
 * set_sp_to_psp()
 * Brief	: Sets the stack pointer to PSP
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
__attribute__((naked)) void set_sp_to_psp(void)
{
	/* 1. Initialize the PSP with Task1_STACK_START --------------------------*/
	__asm volatile("push {lr}"); 	/* Secure LR before making a nested subroutine call */
	__asm volatile("bl get_psp");	/* Get the PSP of the current task */
	__asm volatile("msr psp, r0");	/* Initialize PSP; By AAPCS, r0 will contain Task1_STACK_START */
	__asm volatile("pop {lr}"); 	/* Restore LR after returning from a nested subroutine */

	/* 2. Make PSP the current stack pointer by setting bit[1](SPSEL) of CONTROL register */
	__asm volatile("mov r0, #0x02");
	__asm volatile("msr control, r0");
	__asm volatile("bx lr");
} /* End of set_sp_to_psp */

/* 
 * create_tasks()
 * Brief	: Create four tasks
 * Param	: @t1_handler - Pointer to task1 handler
 * 			: @t2_handler - Pointer to task2 handler
 * 			: @t3_handler - Pointer to task3 handler
 * 			: @t4_handler - Pointer to task4 handler
 * Retval	: None
 * Note		: N/A
 */
void create_tasks(void (*t1_handler)(void),
				  void (*t2_handler)(void),
				  void (*t3_handler)(void),
				  void (*t4_handler)(void))
{
	/* Initialize Task Control Blocks (TCBs) */
	tcbs[0].state = READY;	/* State of the idle task must always be READY */
	tcbs[1].state = READY;
	tcbs[2].state = READY;
	tcbs[3].state = READY;
	tcbs[4].state = READY;

	tcbs[0].psp = IDLE_STACK_START;
	tcbs[1].psp = T1_STACK_START;
	tcbs[2].psp = T2_STACK_START;
	tcbs[3].psp = T3_STACK_START;
	tcbs[4].psp = T4_STACK_START;

	tcbs[0].task_handler = idle_task_handler;
	tcbs[1].task_handler = t1_handler;
	tcbs[2].task_handler = t2_handler;
	tcbs[3].task_handler = t3_handler;
	tcbs[4].task_handler = t4_handler;

	/* ARM Cortex-M4 processor stack model: Full-Descending */

	uint32_t *p_psp;

	/* Create initial dummy contexts */
	for (int i = 0; i < NUM_TASKS; i++)
	{
		p_psp = (uint32_t *)tcbs[i].psp;

		/* xPSR */
		p_psp--;	/* To stay consistent with FD model: Decrement -> store */
		*p_psp = DUMMY_XPSR;	/* 0x01000000; Set T-bit to specify Thumb ISA */

		/* PC */
		p_psp--;
		*p_psp = (uint32_t)tcbs[i].task_handler;
			/* Since ARM Cortex-M4 processor operates only in Thumb state, it's 
			   also a good practice to check if the address of all task handlers
			   is odd. */

		/* LR */
		p_psp--;
		*p_psp = 0xFFFFFFFD;
			/* EXC_RETURN[31:0] - Exception return behavior
			   0xFFFFFFFD: Return to Thread mode, exception return uses 
			   non-floating point state from the PSP and execution uses PSP 
			   after return. -> Matches our environment! */

		/* Set r0-r12 to 0 (optional) */
		/*
		for (int j = 0; j < 13; j++)
		{
			p_psp--;
			// *p_psp = 0; // Don't know why this is setting p_psp to 0. 
			// (Debug needed)
		}
		*/
		p_psp -= 13;	/* space for r0-r12 */

		/* Save the taks's PSP value for later use */
		tcbs[i].psp = (uint32_t)p_psp;
	}
} /* End of create_tasks */

/* 
 * select_next_task()
 * Brief	: Selects the next available task based on the Round-Robin 
 * 			  scheduling policy 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void select_next_task()
{
	int state = BLOCKED;

	/* Loop through the tasks for one cycle to search the schedulable user task */
	for (int i = 0; i < NUM_TASKS; i++)
	{
		curr_task++;
		curr_task %= NUM_TASKS;
		state = tcbs[curr_task].state;

		/* If the task is ready to be scheduled and is not the idle task */
		if ((state == READY) && (curr_task != 0))
		{
			/* Stop searching and return. At this point curr_task contains
			   the handle of the next task to be scheduled. */
			return;
		}
	}

	/* If there was no schedulable user task, select the idle task */
	if (state != READY)
	{
		curr_task = 0;
	}
} /* End of select_next_task */

/* 
 * PendSV_Handler()
 * Brief	: Performs context switching
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
__attribute__((naked)) void PendSV_Handler(void)
{
	/* SF1(r0-r3, r12, lr, pc, xpsr) of the current task are automatically 
	   pushed onto its stack by the processor as exception ENTRY sequence. 
	   (i.e., Stacking) */

	/* CAUTION!
	   You are in an EXCEPTION HANDLER, which uses MSP. When you use push/pop 
	   operation in this handler, MSP will be affected! */

	/***************************************************************************
	 * PART1: Task switching out (Save the context of the current task)
	 **************************************************************************/

	/* 1. Get the current task's PSP */
	__asm volatile("mrs r0, psp");

	/* 2. Save SF2(r4-r11) of the current task by using its PSP */
	__asm volatile("stmdb r0!, {r4-r11}"); 	/* Register to memory */
		/* STMDB: STore Multiple registers. Decrement address Before each access.
		   !	: An optional writeback suffix. If ! is present the final address,
				  that is loaded from or stored to, is written back into the base
				  register. In this case, r0. */

	/* 3. Save the updated PSP of the current task */
	__asm volatile("push {lr}");
		/* Secure LR before making a nested subroutine call. Here LR contains
		   (EXC_RETURN[31:0] - 0xFFFFFFFD) that is updated at the exception 
		   entry sequence. */
	__asm volatile("bl save_psp");

	/***************************************************************************
	 * PART2: Task switching in (Restore the context of the next task)
	 **************************************************************************/

	/* 1. Choose the next task to run */
	__asm volatile("bl select_next_task");

	/* 2. Get the next task's PSP */
	__asm volatile("bl get_psp");	/* Get the PSP of the current task */

	/* 3. Restore SF2(r4-r11) of the next task by using its PSP */
	__asm volatile("ldmia r0!, {r4-r11}");	/* Memory to register */
		/* LDMIA: LoaD Multiple registers. Increment address After each access.
		   !	: An optional writeback suffix. If ! is present the final 
		   		  address, that is loaded from or stored to, is written back 
				  into the base register. In this case, r0. */

	/* 4. Update PSP */
	__asm volatile("msr psp, r0");	/* Now PSP points to the stack of the task switched in */

	__asm volatile("pop {lr}");		/* Restore LR (EXC_RETURN[31:0] - 0xFFFFFFFD) */

	__asm volatile("bx lr");

	/* SF1(r0-r3, r12, lr, pc, xpsr) of the current task are automatically 
	   popped out of its stack and restored by the processor as exception EXIT 
	   sequence. (i.e., UnstackingG) */
} /* End of PendSV_Handler */

/* 
 * unblock_tasks()
 * Brief	: Checks all the tasks' states and unblock all the tasks that are 
 * 			  qualified 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void unblock_tasks(void)
{
	for (int i = 1; i < NUM_TASKS; i++)
	{
		if (tcbs[i].state != READY)
		{
			/* If the blocking time has elapsed */
			if (tcbs[i].block_count == global_tick_count)
			{
				tcbs[i].state = READY;
			}
		}
	}
} /* End of unblock_tasks */

/* 
 * SysTick_Handler()
 * Brief	: Increments the global tick count, unblocks qualified tasks, and 
 * 			  pends the PendSV exception to trigger context switching 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void SysTick_Handler(void)
{
	/* Increment the global tick count */
	global_tick_count++;

	/* Checks all the tasks' states and unblock all the tasks that are qualified */
	unblock_tasks();

	/* Pend the PendSV exception */
	ICSR |= PENDSVSET;
} /* End of SysTick_Handler */

/* 
 * enable_processor_faults()
 * Brief	: Enable all configurable exceptions (i.e., UsageFault, MemManage, 
 * 			  BusFault) 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void enable_processor_faults(void)
{
	SHCSR |= USGFAULTENA;	/* Enable UsageFault */
	SHCSR |= BUSFAULTENA;	/* Enable BusFault */
	SHCSR |= MEMFAULTENA;	/* Enable MemManage */

	/* Since the kernel performs various memory access enabling these faults will help us
	 * track down the issues.
	 */
} /* End of enable_processor_faults *.

/* 
 * HardFault_Handler()
 * Brief	: HardFault handler 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void HardFault_Handler(void)
{
	printf("Exception: HardFault\n");
	while (1);
} /* End of HardFault_Handler() */

/* 
 * MemManage_Handler()
 * Brief	: MemManage handler 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void MemManage_Handler(void)
{
	printf("Exception: HardFault\n");
	while (1);
} /* End of MemManage_Handler() */

/* 
 * BusFault_Handler()
 * Brief	: BusFault handler 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void BusFault_Handler(void)
{

	printf("Exception: BusFault\n");
	while (1);
} /* End of BusFault_Handler() */

/* 
 * start_kernel()
 * Brief	: Does necessary initializations and starts the kernel 
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void start_kernel(void)
{
	enable_processor_faults();

	init_sched_stack(SCHED_STACK_START);

	init_systick_timer(TICK_HZ);

	set_sp_to_psp();

	/* Invoke task1_handler */
	(tcbs[1].task_handler)();
} /* End of start_kernel */
