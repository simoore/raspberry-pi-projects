INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
BUILD_DIR=build

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= lab1.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	lab1

clean:
	-rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	-rm -f $(BUILD_DIR)/lab1

lab1: lab1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
