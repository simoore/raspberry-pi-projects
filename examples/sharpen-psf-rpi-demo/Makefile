INCLUDE_DIRS = 
LIB_DIRS = 
CC = gcc
BUILD_DIR=build

CDEFS=
#CFLAGS= -O0 $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O0 -msse3 -malign-double $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O2 -msse3 -malign-double $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O3 -msse3 $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
CFLAGS= -O3 -mcpu=cortex-a7 -mfpu=neon-vfpv4 $(INCLUDE_DIRS) $(CDEFS)
LIBS=-lpthread

PRODUCT=sharpen_grid sharpen

HFILES= 
CFILES= sharpen_grid.c sharpen.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*

sharpen_grid:	sharpen_grid.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/sharpen_grid.o $(LIBS)

sharpen:	sharpen.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/sharpen.o $(LIBS)

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
