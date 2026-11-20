CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude

SRC = src/main.c src/server.c src/http.c src/mime.c src/files.c
OBJ = $(SRC:.c=.o)
EXEC = minihttpd

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
