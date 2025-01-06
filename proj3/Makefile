CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -D_DEFAULT_SOURCE
LDFLAGS = -pthread

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(LDFLAGS)

clean:
	rm -f server client *.o

.PHONY: all clean