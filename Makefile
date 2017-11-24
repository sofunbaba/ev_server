TARGET=multi_cgminer_server
SRC=$(wildcard *.c)

ARM_GCC_HOME=/opt/gcc-linaro-5.4.1
LIBEVENT=./libevent-2.1.8-stable
CROSS_COMPILE ?= $(ARM_GCC_HOME)/bin/arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

CFLAGS=-Wall -I$(LIBEVENT)/include
LDFLAGS=-L$(LIBEVENT)/.libs
LIBS=-levent -levent_pthreads -lpthread

$(TARGET):$(SRC)
	@$(MAKE) -C $(LIBEVENT)
	@cp -f $(LIBEVENT)/.libs/libevent-2.1.so.6.0.2 ./libevent-2.1.so.6
	@cp -f $(LIBEVENT)/.libs/libevent_pthreads-2.1.so.6.0.2 ./libevent_pthreads-2.1.so.6
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY:clean tags strip

strip:
	$(STRIP) $(TARGET)

clean:
	$(RM) $(TARGET) libevent-2.1.so.6 libevent_pthreads-2.1.so.6

tags:
	@ctags -R *
