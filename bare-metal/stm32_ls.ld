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
		*(.rodata)		/* Merge all .rodata sections from all(*) input files */

		/* Note: The location counter '.' always tracks VMA of the section in which it is
		   being used. (Not the LMA) */
		_etext = .;	/* Store the updated location counter value into the symbol _etext */
	}> FLASH	/* .text section stays in FLASH, so both vma/lma are FLASH.
				   Linker generates load address and virtual address using this statement.
				   >(vma) AT>(lma), when vma=lma you can simply write >(vma) */
	
	/* Initialized data */
	.data :
	{
		_sdata = .;	/* _sdata will now store the start addr of SRAM, 0x20000000 */
		*(.data)
		_edata = .;
	}> SRAM AT> FLASH

	/* Uninitialized data */
	.bss :
	{
		_sbss = .;
		*(.bss)
		_ebss = .;
	}> SRAM 	/* .bss does not need LMA since it does not get loded onto FLASH */
}
