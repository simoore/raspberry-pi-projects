INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
BUILD_DIR=build

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= pthread.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	pthread

clean:
	-rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	-rm -f $(BUILD_DIR)/pthread

run:
	$(BUILD_DIR)/pthread

pthread: pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o -lpthread -lrt

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
