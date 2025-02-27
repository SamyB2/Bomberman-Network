.PHONY: main client clean server

CC = gcc

CFLAGS = -O -Wall -g -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wno-parentheses \
         -Wcast-align -Wconversion -Wformat \
         -Wformat-nonliteral -Wmissing-braces -Wuninitialized \
         -Wmissing-declarations  -Winline \
         -Wmissing-prototypes -Wredundant-decls \
         -Wformat-security -pedantic -Werror -lncurses


SRCS = src/main.c src/board.c src/bomb.c src/pos.c src/dequeue.c src/stack.c src/player.c src/direction.c src/chat.c
SRCS_CLIENT = src/client.c src/format.c src/view.c src/chat.c src/dequeue.c src/board.c src/stack.c
SRCS_SERVER = src/server.c src/udp.c src/format.c src/dequeue.c src/board.c src/stack.c

TARGET = main
CLIENT = client
SERVER = server

main:
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) 

client:
	$(CC) $(SRCS_CLIENT) -o $(CLIENT) $(CFLAGS)

server:
	$(CC) $(SRCS_SERVER) -o $(SERVER) $(CFLAGS)

clean:
	rm -f main client