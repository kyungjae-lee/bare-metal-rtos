/*******************************************************************************
 * File		: main.c
 * Brief	: Application to test RTOS kernel functionality
 * Author	: Kyungjae Lee
 * Date		: 04/11/2023
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "led.h"
#include "kernel.h"

/* Function prototypes */
void task1_handler(void);		/* Task 1 */
void task2_handler(void);		/* Task 2 */
void task3_handler(void);		/* Task 3 */
void task4_handler(void);		/* Task 4 */

extern void initialise_monitor_handles(void);	/* Semihosting init function */

int main(void)
{
	/* Initialize Semihosting for message printing feature */
	initialise_monitor_handles();

	printf("Testing bare-metal RTOS\n");

	/* Initialize LEDs */
	led_init();

	/* Create tasks */
	create_tasks(task1_handler, task2_handler, task3_handler, task4_handler);

	/* Start kernel*/
	start_kernel();

    /* Loop forever */
	for(;;);
} /* End of main */

/* 
 * task1_handler()
 * Brief	: Toggles green LED every 1000 ms
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void task1_handler(void)
{
	while (1)
	{
		printf("Task 1\n");
		led_green_on();
		block_task(1000);
		led_green_off();
		block_task(1000);
	}
} /* End of task1_handler */

/* 
 * task2_handler()
 * Brief	: Toggles orange LED every 500 ms
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void task2_handler(void)
{
	while (1)
	{
		printf("Task 2\n");
		led_orange_on();
		block_task(500);
		led_orange_off();
		block_task(500);
	}
} /* End of task2_handler */

/* 
 * task3_handler()
 * Brief	: Toggles blue LED every 250 ms
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void task3_handler(void)
{
	while (1)
	{
		printf("Task 3\n");
		led_blue_on();
		block_task(250);
		led_blue_off();
		block_task(250);
	}
} /* End of task3_handler */

/* 
 * task4_handler()
 * Brief	: Toggles red LED every 250 ms
 * Param	: None
 * Retval	: None
 * Note		: N/A
 */
void task4_handler(void)
{
	while (1)
	{
		printf("Task 4\n");
		led_red_on();
		block_task(125);
		led_red_off();
		block_task(125);
	}
} /* End of task4_handler */
