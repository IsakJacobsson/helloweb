CC = gcc

CFLAGS = -Wall -Wextra -O2 -I../include

LIB = ../build/libhelloweb.a

TARGET = server_app

SRCS = helloweb_server_port.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LIB)

clean:
	rm -f $(TARGET)
