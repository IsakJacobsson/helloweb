CC = gcc
AR = ar

CFLAGS = -Wall -Wextra -O2 -Iinclude

LIB_NAME = libhelloweb.a
LIB_DIR = build
LIB = $(LIB_DIR)/$(LIB_NAME)

SRCS = src/helloweb.c
OBJS = $(SRCS:.c=.o)


.PHONY: all lib clean

all: lib

lib: $(LIB)

$(LIB): $(OBJS)
	mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -rf $(LIB_DIR)
