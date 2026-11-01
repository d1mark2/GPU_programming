TARGET = det_gpu.out
CXX = clang++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
METAL_CPP_DIR = $(CURDIR)/../gpu
INCLUDES = -I$(METAL_CPP_DIR)
FRAMEWORKS = -framework Metal -framework Foundation -framework QuartzCore

all: $(TARGET) determinant.metallib

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) main.cpp $(FRAMEWORKS) -o $(TARGET)

determinant.metallib: determinant.metal
	xcrun -sdk macosx metal -c determinant.metal -o determinant.air
	xcrun -sdk macosx metallib determinant.air -o determinant.metallib

clean:
	rm -f $(TARGET) determinant.air determinant.metallib

.PHONY: all clean
