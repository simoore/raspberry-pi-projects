CC=gcc
BUILD_DIR=build

#CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O1 -Wall $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O2 -Wall $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O3 -Wall $(INCLUDE_DIRS) $(CDEFS)
CFLAGS=-O3 -Wall -mcpu=cortex-a7 -mfpu=neon-vfpv4 $(INCLUDE_DIRS) $(CDEFS)
LIBS=-lpthread

PRODUCT=erast erastsimp
CFILES=erast.c

all: ${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*

erastsimp:	$(BUILD_DIR)/erastsimp.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/erastsimp.o $(LIBS)

erast:	$(BUILD_DIR)/erast.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/erast.o $(LIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
