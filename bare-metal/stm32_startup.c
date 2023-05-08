#include <stdint.h>

#define SRAM_START	0x20000000U
#define SRAM_SIZE	(128 * 1024)	/* 128 KB */
#define SRAM_END 	((SRAM_START) + (SRAM_SIZE))

#define STACK_START	SRAM_END

/* Function prototypes */
void Reset_Handler(void);

/* Vector table */
/* Due to the nature of the array 'vectors', the compiler will place it in the .data
   section. Since the vector table must be placed in the beginning of the code memory, we
   use 'section' gcc attribute to place it in the user defined section.
   Here, leading '.' is optional. I used it to stay consistent with other section names.
*/
uint32_t vectors[] __attribute__((section(".isr_vector"))) = {
	STACK_START,				/* Stack pointer (MSP) */
	(uint32_t)&Reset_Handler
};
