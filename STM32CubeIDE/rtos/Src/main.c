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


int main(void)
{
	led_init();

	create_tasks(task1_handler, task2_handler, task3_handler, task4_handler);
	start_kernel();

    /* Loop forever */
	for(;;);
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
