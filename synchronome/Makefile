BUILD_DIR=build
CPPFLAGS=-O3 -g -mtune=cortex-a72 -mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -std=gnu++20
LIBS=-lpthread -lrt

SRCS=camera.cpp capture.cpp image_processor.cpp
OBJS=$(addprefix $(BUILD_DIR)/, $(SRCS:.cpp=.o))

all: capture

clean:
	-rm -f $(BUILD_DIR)/*

capture: $(OBJS)
	g++ $(CFLAGS) -o $(BUILD_DIR)/$@ $^ $(LIBS) 

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	g++ -MD $(CPPFLAGS) -c -o $@ $<
