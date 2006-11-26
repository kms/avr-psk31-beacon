#-----------------------------------------------------------------------------
# ARM-GCC standard Makefile
# This makefile is to be used by including it from a project-specific makefile
# which defines the source files and compiler/linker options
#
# Written by Pascal Stang
# Based on Volker Oth's AVR makefiles of jan.2000
# ----------------------------------------------------------------------------
	
	TIMESTAMP = \"`date  --iso-8601=seconds | tr " " T`\"

	MCU	= atmega88

	TRG     = avr-psk31-beacon

	TESTS	= 
	TESTABLES = 
	TESTRUNNER = 

	SRC 	= $(TRG).c $(TESTABLES)

#put additional assembler source file here
	ASRC 	=

#additional libraries and object files to link
	LIB	= -lm

#additional includes to compile
	INC	= fRound.h fifo.h parser.h testSuites.h

#assembler flags
	ASFLAGS = -Wa, -gstabs

#compiler flags
	CPFLAGS	= -g -Os -Wall -Wstrict-prototypes -I$(AVRLIB) -Wa,-ahlms=$(<:.c=.lst) -fpack-struct -fshort-enums -mcall-prologues -D__TIMESTAMP_STRING__=$(TIMESTAMP) -Winline -fwhole-program -combine 

#linker flags
	LDFLAGS = -Wl,-Map=$(TRG).map,--cref -fwhole-program -combine

### BLOCK 1) define some variables based on the AVR base path in $(AVR) ###

	CC	= avr-gcc
	AS	= avr-gcc -x assembler-with-cpp	
	RM	= rm -f
	RN	= mv
	CP	= cp
	BIN	= avr-objcopy
	SIZE	= ./avr-mem.sh
	INCDIR	= .
	AVRLIB	= avrlib

	TESTCC	= gcc

### BLOCK 2) output format can be srec, ihex (avrobj is always created) ####

	FORMAT = ihex

### BLOCK 3) define all project specific object files ###

	SRC	+= $(addprefix $(AVRLIB)/,$(AVRLIB_SRC))
	OBJ	= $(ASRC:.s=.o) $(SRC:.c=.o) 
	CPFLAGS += -mmcu=$(MCU) -g -Os -Wall -Wstrict-prototypes -I$(AVRLIB) -Wa,-ahlms=$(<:.c=.lst)
	ASFLAGS += -mmcu=$(MCU)
	LDFLAGS += -mmcu=$(MCU)
  
### BLOCK 4) this defines the aims of the make process ###

all:	$(TRG).elf $(TRG).hex $(TRG).eep $(TRG).ok


### BLOCK 5) compile: instructions to create assembler and/or object files from C source ###

%.o : %.c
	$(CC) -c $(CPFLAGS) -I$(INCDIR) $(SRC) -o $@

%.s : %.c
	$(CC) -S $(CPFLAGS) -I$(INCDIR) $< -o $@


### BLOCK 6) assemble: instructions to create object file from assembler files ###

%.o : %.s
	$(AS) -c $(ASFLAGS) -I$(INCDIR) $< -o $@


### BLOCK 7)  link: instructions to create elf output file from object files ###
%.elf: $(TRG).o
	$(CC) $(TRG).o $(LIB) $(LDFLAGS) -o $@

### BLOCK 8) create avrobj file from elf output file ###

### BLOCK 9) create bin (.hex and .eep) files from elf output file ###

%.hex: %.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	$(BIN) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

###### BLOCK 10) If all other steps compile ok then echo "Errors: none" ######

%ok: size
	@echo "Errors: none" 

###### BLOCK 11)  make instruction to delete created files ######

clean:
	$(RM) $(OBJ)
	$(RM) $(SRC:.c=.s)
	$(RM) $(SRC:.c=.lst)
	$(RM) $(TRG).map
	$(RM) $(TRG).elf
	$(RM) $(TRG).obj
	$(RM) $(TRG).a90
	$(RM) $(TRG).hex
	$(RM) $(TRG).sym
	$(RM) $(TRG).eep
	$(RM) $(TRG).hex
	$(RM) *.bak
	$(RM) *.log
	$(RM) $(TESTRUNNER)
	@echo "Errors: none"
	
size:
	$(SIZE) $(TRG).elf $(MCU)

up:
	avrdude -p $(MCU) -c stk500v2 -U flash:w:$(TRG).hex -v
#	-U eeprom:w:$(TRG).eep -v
#

test:
	$(TESTCC) -o $(TESTRUNNER) $(TESTABLES) $(TESTS) $(TESTRUNNER).c -lcheck
	./$(TESTRUNNER)
