TARGET = main.out
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
METAL_CPP_DIR = $(CURDIR)/Metal
INCLUDES = -I$(METAL_CPP_DIR)
FRAMEWORKS = -framework Metal -framework Foundation -framework QuartzCore
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(FRAMEWORKS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean