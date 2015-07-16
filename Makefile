TARGET=8da
CC=/usr/bin/gcc
INSTALL=/usr/bin/install
override CFLAGS:=-g --std=c99 -Wall -Wpedantic -Wshadow -Werror $(CFLAGS)
DEPS=
OBJ=$(patsubst %.c, %.o, $(wildcard *.c))

bindir=/usr/local/bin

.PHONY: default all clean install

default: $(TARGET)
all: default

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

install: all
	$(INSTALL) $(TARGET) $(bindir)/$(TARGET)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -rf $(TARGET).dSYM
