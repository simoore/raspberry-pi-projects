INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
AS=as
BUILD_DIR=build

CDEFS=
CFLAGS= -O0 $(INCLUDE_DIRS) $(CDEFS)
ASFLAGS= $(INCLUDE_DIRS)
LIBS= 

HFILES= lcmwrapper.h

CFILES= lcmwrapper.c lcmc.c
ASMFILES= lcmarm.s


all:	lcmc.gen.asm lcmwrapper_c lcmwrapper_asm

clean:
	-rm -f $(BUILD_DIR)/*

distclean:
	-rm -f $(BUILD_DIR)/*


# Uncomment lcmwrapper that uses assembly code or the version that uses the C code
# as needed to test linking the ASM or the C code of the same name

#lcmwrapper_asm: lcmintel.o lcmasmwrapper.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ lcmintel.o lcmasmwrapper.o

lcmwrapper_asm: lcmarm.o lcmasmwrapper.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/lcmarm.o $(BUILD_DIR)/lcmasmwrapper.o

lcmwrapper_c: lcmc.o lcmwrapper.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/lcmc.o $(BUILD_DIR)/lcmwrapper.o


# C code generation of object code
#
lcmc.o: lcmc.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@

lcmwrapper.o: lcmwrapper.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@

lcmasmwrapper.o: lcmasmwrapper.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@


# Assembly code generation by C compiler and creation of object code from 
# hand written assembly lcmarm.s
#
# If the hand written lcmarm.s does not work on your architecture, e.g., you are on x86, then
# you can use the generated ASM lcm.gen.asm to replace lcmarm.s instead.
#
# As an example, I have saved off some Intel architecture generated code as "lcmintel.s".
#
lcmc.gen.asm: lcmc.c
	mkdir -p $(BUILD_DIR)
	$(CC) -O0 -S $< -o $(BUILD_DIR)/$@

lcmarm.o: lcmarm.s
	mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $(BUILD_DIR)/$@

lcmintel.o: lcmintel.s
	mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $(BUILD_DIR)/$@

