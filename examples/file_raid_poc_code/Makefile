BUILD_DIR=build
CFLAGS= -O0 -g -pg
#CFLAGS= -O3 -msse3 -malign-double -g

DRIVER=raidtest raid_perftest stripetest
OBJS=$(BUILD_DIR)/raidlib.o

all: ${DRIVER}

clean:
	-rm -f $(BUILD_DIR)/*

raidtest: ${OBJS} $(BUILD_DIR)/raidtest.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(OBJS) $(BUILD_DIR)/raidtest.o

stripetest:	${OBJS} $(BUILD_DIR)/stripetest.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(OBJS) $(BUILD_DIR)/stripetest.o

raid_perftest:	${OBJS} $(BUILD_DIR)/raid_perftest.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(OBJS) $(BUILD_DIR)/raid_perftest.o

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
