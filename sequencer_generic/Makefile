INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
BUILD_DIR=build

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= seqgenex0.c seqgen.c seqgen2.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	seqgenex0 seqgen seqgen2 clock_times

clean:
	-rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	-rm -f $(BUILD_DIR)/seqgenex0 $(BUILD_DIR)/seqgen $(BUILD_DIR)/seqgen2 $(BUILD_DIR)/clock_times

seqgenex0: seqgenex0.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread -lrt

seqgen2: seqgen2.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread -lrt

seqgen: seqgen.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread -lrt

clock_times: clock_times.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread -lrt

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
