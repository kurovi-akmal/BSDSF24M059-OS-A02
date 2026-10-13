
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = bin/ls
SRC = src/ls-v1.0.0.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
