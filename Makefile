TARGET=evServer
SRC=$(wildcard *.c)

CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

CFLAGS=-Wall
LDFLAGS=-L/usr/lib
LIBS=-levent -levent_pthreads -lpthread

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY:clean tags strip

strip:
	$(STRIP) $(TARGET)

clean:
	$(RM) $(TARGET)

tags:
	ctags -R *

