INCLUDE_DIRS = 
LIB_DIRS = 
BUILD_DIR=build

CDEFS= 
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lpthread -lrt

#PRODUCT=posix_timer
PRODUCT=posix_rt_timer posix_timer itimer posix_sw_wd

HFILES=
CFILES= posix_rt_timer.c posix_timer.c itimer.c posix_sw_wd.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	-rm -f $(BUILD_DIR)/${PRODUCT}

posix_sw_wd:	posix_sw_wd.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_sw_wd.o $(LIBS)

itimer:	itimer.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/itimer.o $(LIBS)

posix_timer:	posix_timer.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_timer.o $(LIBS)

posix_rt_timer:	posix_rt_timer.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/posix_rt_timer.o $(LIBS)

depend:

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) -MD $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
