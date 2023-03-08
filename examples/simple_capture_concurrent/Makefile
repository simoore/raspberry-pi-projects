BUILD_DIR=build
CFLAGS= -O3 -g -std=gnu++20
LIBS= -lpthread -lrt
PRODUCT=heap_mq posix_mq
HFILES=
CFILES= heap_mq.c posix_mq.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all: capture

clean:
	-rm -f $(BUILD_DIR)/*

posix_mq:	posix_mq.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ posix_mq.o $(LIBS)

heap_mq:	heap_mq.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ heap_mq.o $(LIBS)

capture: capture.o
	g++ $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/capture.o $(LIBS)

.cpp.o:
	mkdir -p $(BUILD_DIR)
	g++ -MD $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
