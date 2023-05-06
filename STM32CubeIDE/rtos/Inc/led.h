/*****************************************************************************************
 * @ File name		: led.h
 * @ Description	: Interface for STM32F407 Discovery board LED driver
 * @ Author			: Kyungjae Lee
 * @ Date created	: 04/11/2023
 ****************************************************************************************/

#ifndef LED_H
#define LED_H

#include "stdint.h"

/* User LEDs */
#define LED_GREEN			(1U << 12)	/* PD12 */
#define LED_ORANGE			(1U << 13)	/* PD13 */
#define	LED_RED				(1U << 14)	/* PD14 */
#define LED_BLUE			(1U << 15)	/* PD15 */

#define LED_GREEN_MODE_OUTPUT		(1U << 24)
#define LED_ORANGE_MODE_OUTPUT		(1U << 26)
#define LED_RED_MODE_OUTPUT			(1U << 28)
#define LED_BLUE_MODE_OUTPUT		(1U << 30)

/* RCC */
#define RCC_BASE			0x40023800U
#define RCC_AHB1ENR			(*(uint32_t volatile *)(RCC_BASE + 0x30U))

/* GPIO */
#define GPIOD_BASE			0x40020C00U
#define GPIOD_MODER			(*(uint32_t volatile *)(GPIOD_BASE + 0x00U))
#define GPIOD_ODR			(*(uint32_t volatile *)(GPIOD_BASE + 0x14U))
#define GPIODEN				(1U << 3)	/* IO port D clock enable */

/* Delay */
#define DELAY_COUNT_1MS		1250U
#define DELAY_COUNT_125MS	(125U * DELAY_COUNT_1MS)
#define DELAY_COUNT_250MS	(250U * DELAY_COUNT_1MS)
#define DELAY_COUNT_500MS	(500U * DELAY_COUNT_1MS)
#define DELAY_COUNT_1000MS	(1000U * DELAY_COUNT_1MS)

/* LED interface */
void led_init(void);

void led_green_on(void);
void led_green_off(void);
void led_green_toggle(void);

void led_orange_on(void);
void led_orange_off(void);
void led_orange_toggle(void);

void led_red_on(void);
void led_red_off(void);
void led_red_toggle(void);

void led_blue_on(void);
void led_blue_off(void);
void led_blue_toggle(void);

void delay(uint32_t count);

#endif
