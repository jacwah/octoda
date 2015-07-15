TARGET=8da
CC=gcc
override CFLAGS:=-g --std=c99 -Wall -Wpedantic -Wshadow -Werror $(CFLAGS)
DEPS=
OBJ=$(patsubst %.c, %.o, $(wildcard *.c))

.PHONY: default all clean

default: $(TARGET)
all: default

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -rf $(TARGET).dSYM
