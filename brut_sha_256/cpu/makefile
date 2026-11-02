TARGET = brut_sha_256.out
CC = clang
CFLAGS = -Wall -Wextra -g
SRCS = sha_256.c brut_256.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET) sha_256.out

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

sha_256.out: get_hash.o sha_256.o
	$(CC) $(CFLAGS) get_hash.o sha_256.o -o sha_256.out

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean