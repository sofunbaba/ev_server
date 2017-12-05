TARGET=multi_cgminer_server
VERSION_STRING="1.0"

SRC=$(wildcard src/*.c) $(wildcard libs/ccan/*.c) $(wildcard libs/jansson/*.c)

ARM_GCC_HOME=/opt/gcc-linaro-5.4.1
LIBEVENT=libs/libevent-2.1.8-stable
CROSS_COMPILE ?= $(ARM_GCC_HOME)/bin/arm-linux-gnueabihf-

export CC = $(CROSS_COMPILE)gcc
export STRIP = $(CROSS_COMPILE)strip

CFLAGS=-I$(LIBEVENT)/include -I$(LIBJANSSON)/src -I$(LIBJANSSON)/include -Ilibs/ccan -Ilibs/jansson -I. -g
LDFLAGS=-L$(LIBEVENT)/.libs
LIBS=-levent -levent_pthreads -lpthread

$(TARGET): version.h $(SRC)
	@$(MAKE) -C $(LIBEVENT)
	@cp -f $(LIBEVENT)/.libs/libevent-2.1.so.6.0.2 ./libevent-2.1.so.6
	@cp -f $(LIBEVENT)/.libs/libevent_pthreads-2.1.so.6.0.2 ./libevent_pthreads-2.1.so.6
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY:clean tags strip

strip:
	$(STRIP) $(TARGET)

clean:
	$(RM) $(TARGET) libevent-2.1.so.6 libevent_pthreads-2.1.so.6 version.h

tags:
	@ctags -R *

version.h:
	@`touch version.h`
	@echo "#define DEFAULT_PROGRAM_NAME   \"$(TARGET)\"" > version.h
	@echo "#define DEFAULT_SERVER_VERSION \"$(VERSION_STRING)\"" >> version.h
	@echo "#define DEFAULT_COMPILE_DATE   \"$(shell date)\"" >> version.h
	@echo "#define DEFAULT_COMPILE_OWNER  \"$(shell whoami)\"" >> version.h
