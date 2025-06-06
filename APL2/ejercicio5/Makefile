# Unified Makefile for client and server

CC = gcc
CFLAGS = -Wall -Wextra -O2

CLIENT_SRC = cliente.c
SERVER_SRC = server.c

CLIENT_BIN = cliente
SERVER_BIN = server

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)

clean:
	rm -f $(CLIENT_BIN) $(SERVER_BIN)
