/*****************************************************************************************
 * @ File name		: kernel.h
 * @ Description	: Interface for RTOS kernel
 * @ Author			: Kyungjae Lee
 * @ Date created	: 05/04/2023
 ****************************************************************************************/

#ifndef KERNEL_H
#define KERNEL_H

void kernel_init();
void create_tasks(void (*t1_handler)(void),
				  void (*t2_handler)(void),
				  void (*t3_handler)(void),
				  void (*t4_handler)(void));



#endif
