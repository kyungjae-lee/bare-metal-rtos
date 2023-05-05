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



int main(void)
{
	led_init();

	kernel_init();

    /* Loop forever */
	for(;;);
}

