# Makefile (or makefile)

# Variables
CC=arm-none-eabi-gcc
MACH=cortex-m4
CFLAGS= -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0
LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=nano.specs -T stm32_ls.ld -Wl,-Map=final.map
	# --spec=nano.specs: Link the project with newlib nano C standard library.
	# 					 Cannot be used with -nostdlib at the same time.
	# -mcpu=$(MACH) -mthumb: Must be included in the linker flags as well.
	# -mfloat-abi=soft: Specifying the use of software floating point (Not using the
	# 					hardware FPU)
LDFLAGS_SH= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=rdimon.specs -T stm32_ls.ld -Wl,-Map=final.map
	# Linker flags for semihosting (Here, rdimon.specs must be used instead of nano.specs)

all: main.o kernel.o led.o stm32_startup.o syscalls.o final.elf

# For semihosting
sh: main.o kernel.o led.o stm32_startup.o final_sh.elf
	# Now the library is providing the low-level system calls, so do NOT include
	# syscalls.o!

main.o: main.c						# Target: Dependencies
	$(CC) $(CFLAGS) -o $@ $^		# Recipie
	# $(CC) $(CFLAGS) $^ -o $@
	# $(CC) $(CFLAGS) main.c -o main.o
	# '$^' represents dependency, '$@' represents target (@ does look like a target :))

kernel.o: kernel.c
	$(CC) $(CFLAGS) -o $@ $^

led.o: led.c
	$(CC) $(CFLAGS) -o $@ $^

stm32_startup.o: stm32_startup.c
	$(CC) $(CFLAGS) -o $@ $^

syscalls.o: syscalls.c
	$(CC) $(CFLAGS) -o $@ $^

final.elf: main.o kernel.o led.o stm32_startup.o syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

# For semihosting
final_sh.elf: main.o kernel.o led.o stm32_startup.o 
	$(CC) $(LDFLAGS_SH) -o $@ $^
	# Now the library is providing the low-level system calls, so do NOT include
	# syscalls.o!

clean:
	rm -rf *.o *.elf 		# In windows rm -> del

connect:
	openocd -f /board/stm32f4discovery.cfg
	# /usr/share/openocd/scripts/board/stm32f4discovery.cfg
