CC = gcc

CFLAGS = -Wall -Wextra -O2 -I../include

LIB = ../build/libhelloweb.a

TARGET = server_app

SRCS = helloweb_server_port.c

.PHONY: all clean react

all: react $(TARGET)

$(TARGET): $(SRCS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LIB)

clean:
	rm -f $(TARGET)

# ------------------------
# React build via Docker
# ------------------------

REACT_DIR = react

react:
	docker run --rm \
		-v $(PWD)/$(REACT_DIR):/app \
		-w /app \
		node:20 \
		sh -c "npm install && npm run build"
