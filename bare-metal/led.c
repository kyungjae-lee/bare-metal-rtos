/*****************************************************************************************
 * @ File name		: led.c
 * @ Description	: STM32F407 Discovery board LED driver
 * @ Author			: Kyungjae Lee
 * @ Date created	: 04/11/2023
 ****************************************************************************************/

#include "led.h"

/*****************************************************************************************
 * LED driver Implementation Guideline
 *
 * 1. Identify the GPIO port LED is connected to
 * 2. Identify the bus interface the GPIO peripheral is connected to
 * 3. Enable clock for the GPIO peripheral
 * 4. Identify the GPIO pin number of the LED
 * 5. Configure corresponding pin to output mode
 * 6. Set pin bit to turn on, clear to turn off the LED
 ****************************************************************************************/

void led_init(void)
{
	//__disable_irq();

	/* Enable clock for GPIOD peripheral */
	RCC_AHB1ENR |= GPIODEN;

	/* Set LED pin mode to output mode */
	/* Clear */
	GPIOD_MODER &= ~(0xFFU << 24);
	/* Set */
	GPIOD_MODER |= (LED_GREEN_MODE_OUTPUT |
					LED_ORANGE_MODE_OUTPUT |
					LED_RED_MODE_OUTPUT |
					LED_BLUE_MODE_OUTPUT);

	//__enable_irq();
}

/*
 * Green LED
 */

void led_green_on(void)
{
	GPIOD_ODR |= LED_GREEN;
}

void led_green_off(void)
{
	GPIOD_ODR &= ~LED_GREEN;
}

void led_green_toggle(void)
{
	GPIOD_ODR ^= LED_GREEN;
}

/*
 * Orange LED
 */

void led_orange_on(void)
{
	GPIOD_ODR |= LED_ORANGE;
}

void led_orange_off(void)
{
	GPIOD_ODR &= ~LED_ORANGE;
}

void led_orange_toggle(void)
{
	GPIOD_ODR ^= LED_ORANGE;
}

/*
 * Red LED
 */

void led_red_on(void)
{
	GPIOD_ODR |= LED_RED;
}

void led_red_off(void)
{
	GPIOD_ODR &= ~LED_RED;
}

void led_red_toggle(void)
{
	GPIOD_ODR ^= LED_RED;
}

/*
 * Blue LED
 */

void led_blue_on(void)
{
	GPIOD_ODR |= LED_BLUE;
}

void led_blue_off(void)
{
	GPIOD_ODR &= ~LED_BLUE;
}

void led_blue_toggle(void)
{
	GPIOD_ODR ^= LED_BLUE;
}

/* Spinlock delay */
void delay(uint32_t count)
{
	for (uint32_t i = 0; i < count; i++);
}

