/* Set the entry point */
ENTRY(Reset_Handler)

/* Define the different memories present in the target and their start address and size */
MEMORY
{
	
	/* CODE memory */
	FLASH(rx): ORIGIN =0x08000000, LENGTH =1024K	/* a space is necessary before '=' */
	/* DATA memory */
	SRAM(rwx): ORIGIN =0x20000000, LENGTH =128K		
	/*
	SRAM1(rwx): ORIGIN=0x20000000, LENGTH=116K
	SRAM2(rwx): ORIGIN=0x20000000+116K-4, LENGTH=16
	*/
}

/* Create different output sections and their order in the final ELF executable */
SECTIONS
{
	/* Code */
	.text : 			/* Section name and the ':' must be separated by space */ 
	{
		*(.isr_vector)	/* Put vector table at the beginning of code memory */
		*(.text)		/* Merge all .text sections from all(*) input files */
		*(.text.*)		/* merge if anything like '.text.<function_name>' gets generated */	
		*(.rodata)		/* Merge all .rodata sections from all(*) input files */
		*(.rodata.*)
		*(.init) 		/* merge c standard library specific section into .text (not mandatory)*/
		*(.fini) 		/* merge c standard library specific section into .text (not mandatory) */
		. = ALIGN(4);	/* The location counter will be updated to the word (4 byte)
						   aligned address. This is to force the word-alignment of the
						   section ending. (Word-alignment of the section beginning will
						   be done automatically by the linker script (*fill*), but it
						   does not care about the alignment of the section ending.
						   Therefore, it is good practice for us to align it manually. */

		/* Note: The location counter '.' always tracks VMA of the section in which it is
		   being used. (Not the LMA) */
		_etext = .;	/* Store the updated location counter value into the symbol _etext */
	}> FLASH	/* .text section stays in FLASH, so both vma/lma are FLASH.
				   Linker generates load address and virtual address using this statement.
				   >(vma) AT>(lma), when vma=lma you can simply write >(vma) */
    			/* linker generates absolute address for VMA, LMA */
	
	_la_data = LOADADDR(.data);	/* LOADADDR is a linker command. The load address of .data
								   section will be stored into the symbol _la_data. */

	/* Initialized data */
	.data :
	{
		_sdata = .;	/* _sdata will now store the start addr of SRAM, 0x20000000 */
		*(.data)
		*(.data.*)
		. = ALIGN(4);	/* Force word-boundary alignment for the section ending */
		_edata = .;
	}> SRAM AT> FLASH	/* VMA = SRAM, LMA = FLASH */

	/* Uninitialized data */
	.bss :
	{
		_sbss = .;
		__bss_start__ = _sbss;
		*(.bss)
		*(.bss.*)
        *(COMMON) /* While analyzing the map file you found some data you epxpected to
                     be placed in the .bss section but somehow placed under a COMMON 
                     section created by the linker. This is how you bring it into
                     the the section you want it to be placed in. */
		. = ALIGN(4);	/* Force word-boundary alignment for the section ending */
		_ebss = .;
		__bss_end__ = _ebss;
		. = ALIGN (4);
		end = .;	/* Added to resolve an error regarding the _sbrk in syscalls.c */
		__end__ = .; /* Added to resolve an error regarding semihosting (semihosting
						library needs this linker symbol */
	}> SRAM 	/* .bss does not need LMA since it does not get loded onto FLASH */
}
