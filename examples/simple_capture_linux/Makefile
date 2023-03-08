CC=gcc
BUILD_DIR=build
CFLAGS= -O0 -g
LIBS= -lrt

CFILES= capture.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	capture

clean:
	-rm -f $(BUILD_DIR)/*

distclean:
	-rm -f $(BUILD_DIR)/*

capture: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o $(LIBS)

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
