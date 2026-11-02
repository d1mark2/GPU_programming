TARGET = det_cpu.out
CC = clang
CFLAGS = -O2 -Wall -Wextra

all: $(TARGET)

$(TARGET): det_cpu.c
	$(CC) $(CFLAGS) det_cpu.c -lm -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
