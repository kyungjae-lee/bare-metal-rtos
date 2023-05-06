/*****************************************************************************************
 * @ File name		: kernel.h
 * @ Description	: Interface for RTOS kernel
 * @ Author			: Kyungjae Lee
 * @ Date created	: 05/04/2023
 ****************************************************************************************/

#ifndef KERNEL_H
#define KERNEL_H

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

/* Kernel interface */
void start_kernel(void);
void create_tasks(void (*t1_handler)(void),
				  void (*t2_handler)(void),
				  void (*t3_handler)(void),
				  void (*t4_handler)(void));
void block_task(uint32_t tick_count);

#endif
