CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = source
INC_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)

TARGET = web-server

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

clean:
	rm -f $(TARGET) 

