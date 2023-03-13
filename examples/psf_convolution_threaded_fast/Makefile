CC = gcc
BUILD_DIR=build

#CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O1 $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O2 $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
CFLAGS= -O3 -mcpu=cortex-a7 -mfpu=neon-vfpv4
LIBS=-lpthread

PRODUCT=sharpen_grid sharpen
CFILES= sharpen_grid.c sharpen.c

OBJS=$(BUILD_DIR)/sharpen_grid.c $(BUILD_DIR)/sharpen.c

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*

sharpen_grid: $(BUILD_DIR)/sharpen_grid.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/sharpen_grid.o $(LIBS)

sharpen: $(BUILD_DIR)/sharpen.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/sharpen.o $(LIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
