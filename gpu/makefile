TARGET = brut_sha_256.out
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
METAL_CPP_DIR = $(CURDIR)/Metal
INCLUDES = -I$(METAL_CPP_DIR)
FRAMEWORKS = -framework Metal -framework Foundation -framework QuartzCore
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) metal

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(FRAMEWORKS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

metal: sha_256.metal
	xcrun -sdk macosx metal -c sha_256.metal -o sha_256.air
	xcrun -sdk macosx metallib sha_256.air -o sha_256.metallib

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean