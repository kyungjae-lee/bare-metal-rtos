/*****************************************************************************************
 * @ File name		: kernel.c
 * @ Description	: Implementation of RTOS kernel
 * @ Author			: Kyungjae Lee
 * @ Date created	: 05/04/2023
 ****************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "led.h"

/* Stack memory information */
#define SIZE_TASK_STACK		1024U
#define SIZE_SCHED_STACK	1024U
#define SRAM_START			0x20000000U
#define SIZE_SRAM			((128) * (1024))
#define SRAM_END			((SRAM_START) + (SIZE_SRAM))

#define IDLE_STACK_START	SRAM_END
#define T1_STACK_START		((SRAM_END) - (1 * (SIZE_TASK_STACK)))
#define T2_STACK_START		((SRAM_END) - (2 * (SIZE_TASK_STACK)))
#define T3_STACK_START		((SRAM_END) - (3 * (SIZE_TASK_STACK)))
#define T4_STACK_START		((SRAM_END) - (4 * (SIZE_TASK_STACK)))
#define SCHED_STACK_START	((SRAM_END) - (5 * (SIZE_TASK_STACK)))

/* System timer registers */
/* SysTick Reload Value Register (Stores 24-bit down counter START value) */
#define SYST_RVR			(*(uint32_t volatile *)0xE000E014)
/* SysTick Control and Status Register */
#define SYST_CSR			(*(uint32_t volatile *)0xE000E010)
#define ENABLE				(1 << 0U) /* Counter enabled */
#define TICKINT				(1 << 1U) /* Counting down to zero asserts the SysTick exception request */
#define CLKSOURCE			(1 << 2U) /* Processor clock */

/* PendSV */
/* Interrupt Control and State Register (ICSR) */
#define ICSR				(*(uint32_t volatile *)0xE000ED04)
#define PENDSVSET			(1 << 28U)	/* Change PendSV exception state to pending */

/* Disable interrupts */
#define DISABLE_INTERRUPTS()	do { __asm volatile ("CPSID i"); } while (0)
	/* To disable interrupts for ARM Cortex-M4 processor, you can use the "CPSID i" assembly instruction.
	   This instruction sets the "PRIMASK" register to disable all interrupts, including the non-maskable
	   interrupt (NMI). */
	/* The do-while loop construct ensures that the macro expands to a single statement,
	   even if it is used in a context where multiple statements are expected
	   (e.g., as the body of an if statement). The while (0) part of the loop is there to ensure that
	   the macro can be used safely in compound statements without generating syntax errors. */
	/* Another way of writing DISABLE_INTERRUPTS() is as follows:
	   do { __asm volatile ("mov r0, #0x1"); asm volatile ("mrs primask, r0"); } while (0)
	   In this case, using do-while statement helps ensuring that the compound statements in the body
	   expand to a single statement, as we put parens around every single macro variables. */
/* Enable interrupts */
#define ENABLE_INTERRUPTS() 	do { __asm volatile ("CPSIE i"); } while (0)
	/* Another way of writing DISABLE_INTERRUPTS() is as follows:
	   do { __asm volatile ("mov r0, #0x0"); asm volatile ("mrs primask, r0"); } while (0)  */

/* Clock */
#define HSI_CLK				16000000U
#define SYSTICK_TIM_CLK		HSI_CLK		/* By default */

/* SysTick Timer */
#define TICK_HZ				1000U	/* Desired tick frequency */

#define NUM_TASKS			5U
#define DUMMY_XPSR			0x01000000U	/* Guarantee T-bit is set */

/* System Handler Control and State Register (SHCRS); one of the System Control Block registers */
#define	SHCSR				(*(uint32_t volatile *)0xE000ED24)
#define USGFAULTENA			(1 << 18U)
#define BUSFAULTENA			(1 << 17U)
#define MEMFAULTENA			(1 << 16U)
/* UsageFault Status Register (UFSR) - 16-bit register */
#define UFSR				(*(uint32_t volatile *)0xE000ED2A)
/* Configuration and Control Register */
#define CCR					(*(uint32_t volatile *)0xE000ED14)
#define DIV_0_TRP			(1 << 4U)

/* Task States */
#define READY				0x00U
#define BLOCKED				0xFFU

/* Function prototypes */
void idle_task_handler(void);	/* Task 1 */
void task1_handler(void);		/* Task 1 */
void task2_handler(void);		/* Task 2 */
void task3_handler(void);		/* Task 3 */
void task4_handler(void);		/* Task 4 */

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

TCB_t tcbs[NUM_TASKS];

void schedule(void)
{
	/* Pend the PendSV exception */
	ICSR |= PENDSVSET;
}

/* Blocks the task that calls this function for tick_count ticks */
void task_delay(uint32_t tick_count)
{
	/* To prevent race condition on global variables, make sure to globally disable interrupt */
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
}

/* Initializes SysTick Timer */
void systick_timer_init(uint32_t tick_hz)
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
}

/* Initializes stack of scheduler */
__attribute__((naked)) void sched_stack_init(uint32_t sched_top_of_stack)
{
	//__asm volatile("msr msp, r0");
	__asm volatile("msr msp, %0": : "r" (sched_top_of_stack) : );
	__asm volatile("bx lr");	/* Return to the caller */
}

uint32_t get_psp(void)
{
	return tcbs[curr_task].psp;
}

void save_psp(uint32_t curr_psp)
{
	tcbs[curr_task].psp = curr_psp;
}

__attribute__((naked)) void set_sp_to_psp(void)
{
	/* 1. Initialize the PSP with Task1_STACK_START */
	__asm volatile("push {lr}"); 	/* Secure LR before making a nested subroutine call */
	__asm volatile("bl get_psp");	/* Get the PSP of the current task */
	__asm volatile("msr psp, r0");	/* Initialize PSP; By AAPCS, r0 will contain Task1_STACK_START */
	__asm volatile("pop {lr}"); 	/* Restore LR after returning from a nested subroutine */

	/* 2. Make PSP the current stack pointer by setting bit[1](SPSEL) of CONTROL register */
	__asm volatile("mov r0, #0x02");
	__asm volatile("msr control, r0");
	__asm volatile("bx lr");
}

/* Initializes tasks */
void tasks_init()
{
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
	tcbs[1].task_handler = task1_handler;
	tcbs[2].task_handler = task2_handler;
	tcbs[3].task_handler = task3_handler;
	tcbs[4].task_handler = task4_handler;

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
			/* Since ARM Cortex-M4 processor operates only in Thumb state, it's also a
			   good practice to check if the address of all task handlers is odd. */

		/* LR */
		p_psp--;
		*p_psp = 0xFFFFFFFD;
			/* EXC_RETURN[31:0] - Exception return behavior
			   0xFFFFFFFD: Return to Thread mode, exception return uses non-floating point state from the PSP
			   and execution uses PSP after return. -> Matches our environment! */

		/* Set r0-r12 to 0 */
		for (int j = 0; j < 13; j++)
		{
			p_psp--;
			*p_psp = 0;
		}

		/* Save the taks's PSP value for later use */
		tcbs[i].psp = (uint32_t)p_psp;
	}
}

/* Selects the next available task based on the Round-Robin scheduling policy */
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
}

__attribute__((naked)) void PendSV_Handler(void)
{
	/* SF1(r0-r3, r12, lr, pc, xpsr) of the current task are automatically pushed onto its
	   stack by the processor as exception ENTRY sequence. (i.e., Stacking) */

	/* CAUTION!
	   You are in an EXCEPTION HANDLER, which uses MSP. When you use push/pop operation
	   in this handler, MSP will be affected! */

	/*************************************************************************************
	 * PART1: Task switching out (Save the context of the current task)
	 ************************************************************************************/

	/* 1. Get the current task's PSP */
	__asm volatile("mrs r0, psp");

	/* 2. Save SF2(r4-r11) of the current task by using its PSP */
	__asm volatile("stmdb r0!, {r4-r11}"); 	/* Register to memory */
		/* STMDB: STore Multiple registers. Decrement address Before each access.
		   !	: An optional writeback suffix. If ! is present the final address,
				  that is loaded from or stored to, is written back into the base register.
				  In this case, r0. */

	/* 3. Save the updated PSP of the current task */
	__asm volatile("push {lr}");
		/* Secure LR before making a nested subroutine call. Here LR contains
		   (EXC_RETURN[31:0] - 0xFFFFFFFD) that is updated at the exception engry sequence. */
	__asm volatile("bl save_psp");

	/*************************************************************************************
	 * PART2: Task switching in (Restore the context of the next task)
	 ************************************************************************************/

	/* 1. Choose the next task to run */
	__asm volatile("bl select_next_task");

	/* 2. Get the next task's PSP */
	__asm volatile("bl get_psp");	/* Get the PSP of the current task */

	/* 3. Restore SF2(r4-r11) of the next task by using its PSP */
	__asm volatile("ldmia r0!, {r4-r11}");	/* Memory to register */
		/* LDMIA: LoaD Multiple registers. Iecrement address After each access.
		   !	: An optional writeback suffix. If ! is present the final address,
				  that is loaded from or stored to, is written back into the base register.
				  In this case, r0. */

	/* 4. Update PSP */
	__asm volatile("msr psp, r0");	/* Now PSP points to the stack of the task switched in */

	__asm volatile("pop {lr}");		/* Restore LR (EXC_RETURN[31:0] - 0xFFFFFFFD) */

	__asm volatile("bx lr");

	/* SF1(r0-r3, r12, lr, pc, xpsr) of the current task are automatically popped out of its
	   stack and restored by the processor as exception EXIT sequence. (i.e., UnstackingG) */
}

void update_global_tick_count(void)
{
	global_tick_count++;
}

/* Checks all the tasks' states and unblock all the tasks that are qualified */
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
}

void SysTick_Handler(void)
{
	update_global_tick_count();

	/* Checks all the tasks' states and unblock all the tasks that are qualified */
	unblock_tasks();

	/* Pend the PendSV exception */
	ICSR |= PENDSVSET;
}

/* Enable all configurable exceptions (i.e., UsageFault, MemManage, BusFault) */
void enable_processor_faults(void)
{
	SHCSR |= USGFAULTENA;	/* Enable UsageFault */
	SHCSR |= BUSFAULTENA;	/* Enable BusFault */
	SHCSR |= MEMFAULTENA;	/* Enable MemManage */

	/* Since the kernel performs various memory access enabling these faults will help us
	 * track down the issues.
	 */
}

/* HardFault handler */
void HardFault_Handler(void)
{
	printf("Exception: HardFault\n");
	while (1);
}

/* MemManage handler */
void MemManage_Handler(void)
{
	printf("Exception: HardFault\n");
	while (1);
}

/* BusFault handler */
void BusFault_Handler(void)
{

	printf("Exception: BusFault\n");
	while (1);
}

void kernel_init(void)
{
	enable_processor_faults();

	sched_stack_init(SCHED_STACK_START);

	tasks_init();

	systick_timer_init(TICK_HZ);

	set_sp_to_psp();

	task1_handler();
}

/* Idle task handler */
void idle_task_handler(void)
{
	/* Idle task will run only when all other user tasks are in BLOCKED state */
	while (1);
}

/* Task 1 handler */
void task1_handler(void)
{
	while (1)
	{
		led_green_on();
		task_delay(1000);
		led_green_off();
		task_delay(1000);
	}
}

/* Task 2 handler */
void task2_handler(void)
{
	while (1)
	{
		led_orange_on();
		task_delay(500);
		led_orange_off();
		task_delay(500);
	}
}

/* Task 3 handler */
void task3_handler(void)
{
	while (1)
	{
		led_blue_on();
		task_delay(250);
		led_blue_off();
		task_delay(250);
	}
}

/* Task 4 handler */
void task4_handler(void)
{
	while (1)
	{
		led_red_on();
		task_delay(125);
		led_red_off();
		task_delay(125);
	}
}

