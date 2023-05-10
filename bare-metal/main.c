/*****************************************************************************************
 * @ File name		: main.c
 * @ Description	: Test driver RTOS kernel
 * @ Author			: Kyungjae Lee
 * @ Date created	: 04/11/2023
 ****************************************************************************************/

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
}

/* Task 1 handler */
void task1_handler(void)
{
	/* Toggle green LED every 1000 ms */
	while (1)
	{
		printf("Task 1\n");
		led_green_on();
		block_task(1000);
		led_green_off();
		block_task(1000);
	}
}

/* Task 2 handler */
void task2_handler(void)
{
	/* Toggle orange LED every 500 ms */
	while (1)
	{
		printf("Task 2\n");
		led_orange_on();
		block_task(500);
		led_orange_off();
		block_task(500);
	}
}

/* Task 3 handler */
void task3_handler(void)
{
	/* Toggle blue LED every 250 ms */
	while (1)
	{
		printf("Task 3\n");
		led_blue_on();
		block_task(250);
		led_blue_off();
		block_task(250);
	}
}

/* Task 4 handler */
void task4_handler(void)
{
	/* Toggle red LED every 125 ms */
	while (1)
	{
		printf("Task 4\n");
		led_red_on();
		block_task(125);
		led_red_off();
		block_task(125);
	}
}
