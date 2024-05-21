CC = gcc
CFLAGS = -Wall -Wextra -std=c99

build:
	$(CC) *.c *.h -o client $(CFLAGS)

clean:
	rm -f client
.PHONY: clean
	