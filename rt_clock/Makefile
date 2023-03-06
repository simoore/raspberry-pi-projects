INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
BUILD_DIR=build

CDEFS= 
CFLAGS= -O3 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

PRODUCT=posix_clock

HFILES=
CFILES= posix_clock.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	-rm -f $(BUILD_DIR)/${PRODUCT}

posix_clock:	posix_clock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_clock.o $(LIBS)

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) -MD $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
