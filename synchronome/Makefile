BUILD_DIR=build
CPPFLAGS=-O3 -g -mtune=cortex-a72 -mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -std=gnu++20
LIBS=-lpthread -lrt
INCLUDES=-I../third_party/popl/include

SRCS=camera_service.cpp camera.cpp image_saver_service.cpp image_saver.cpp main.cpp service.cpp \
	tick_detector_service.cpp tick_detector.cpp
OBJS=$(addprefix $(BUILD_DIR)/, $(SRCS:.cpp=.o))

all: build/synchronome

clean:
	rm -f $(BUILD_DIR)/*

build/synchronome: $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LIBS) 

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	g++ -MD $(CPPFLAGS) $(INCLUDES) -c -o $@ $<
