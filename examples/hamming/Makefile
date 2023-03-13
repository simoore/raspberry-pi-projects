BUILD_DIR=build
CC=gcc
CFLAGS=-O0 -g
#CFLAGS= -O3 -Wall -pg -msse3 -malign-double -g

DRIVER=ecctest

HFILES=ecclib.h
CFILES=ecctest.c ecclib.c

SRCS= ${HFILES} ${CFILES}
OBJS=$(addprefix $(BUILD_DIR)/, $(CFILES:.c=.o))

all:	${DRIVER}

clean:
	-rm -f ${BUILD_DIR)/*

${DRIVER}:	${OBJS}
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@