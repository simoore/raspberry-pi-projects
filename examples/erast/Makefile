CC = gcc
CFLAGS= -O3
#CFLAGS= -O3 -msse3 -malign-double $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread
BUILD_DIR=build
PRODUCT=erast
CFILES= erast.c
OBJS= $(BUILD_DIR)/erast.o

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*

${PRODUCT}:	${OBJS}
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(OBJS) $(LIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@