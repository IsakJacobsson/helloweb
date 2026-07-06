CC = gcc
INCLUDES = -I../include

CFLAGS = -Wall -Wextra -O2 $(INCLUDES)

SRCS = helloweb_server_port.c
OBJS = $(SRCS:.c=.o)

TARGET = server_app

LDFLAGS = -L../lib
LDLIBS = -lhelloweb

.PHONY: all clean libhelloweb

all: libhelloweb $(TARGET)

libhelloweb:
	$(MAKE) -C ..

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET)
	$(MAKE) -C .. clean
