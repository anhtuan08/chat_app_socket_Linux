CC = gcc
CFLAGS = -Wall -g
LIBS = -lpthread

SERVER_SRC = server_2.c
CLIENT_SRC = client.c
COMMON_SRC = common.c

SERVER_OBJ = $(SERVER_SRC:.c=.o) $(COMMON_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o) $(COMMON_SRC:.c=.o)

all: server client

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o server_2 $(SERVER_OBJ) $(LIBS)

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o client $(CLIENT_OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o server client

.PHONY: all clean