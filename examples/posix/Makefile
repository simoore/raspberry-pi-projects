NCLUDE_DIRS = 
LIB_DIRS = 
BUILD_DIR=build

CDEFS= 
CFLAGS= -O3 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

PRODUCT=posix_clock posix_linux_demo posix_mq signal_demo

HFILES=
CFILES= posix_clock.c posix_linux_demo.c posix_mq.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*

posix_linux_demo:	posix_linux_demo.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_linux_demo.o $(LIBS)

posix_clock:	posix_clock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_clock.o $(LIBS)

posix_mq:	posix_mq.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_mq.o $(LIBS)

signal_demo:	signal_demo.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/signal_demo.o $(LIBS)

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) -MD $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
