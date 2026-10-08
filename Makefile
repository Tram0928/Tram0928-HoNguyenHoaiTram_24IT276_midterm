CC=cc

CFLAGS=-Wall -Wextra -std=c99 -Iinclude

SRC=src/main.c \
    src/parser.c \
    src/listing.c \
    src/sorting.c \
    src/display.c \
    src/recursion.c \
    src/utils.c

OBJ=$(SRC:.c=.o)

TARGET=myls

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
