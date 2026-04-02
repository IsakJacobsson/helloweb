# Makefile for your C project

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Source files
SRCS = basic_helloweb_server.c helloweb.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = basic_webserver

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
