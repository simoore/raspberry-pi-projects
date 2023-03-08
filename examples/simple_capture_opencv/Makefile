CC=g++
BUILD_DIR=build
CFLAGS= -O0 -g
LIBS= -lrt
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video

CPPFILES= capture.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	capture 

clean:
	-rm -f $(BUILD_DIR)/*

distclean:
	-rm -f $(BUILD_DIR)/*

capture: capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.o `pkg-config --libs opencv4` $(CPPLIBS)

.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) `pkg-config --cflags opencv4` -c $< -o $(BUILD_DIR)/$@

.cpp.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) `pkg-config --cflags opencv4` -c $< -o $(BUILD_DIR)/$@
