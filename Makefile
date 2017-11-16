TARGET=server
SRC=$(wildcard *.c)

CFLAGS=
LDFLAGS=-L/usr/lib
LIBS=-levent -levent_pthreads -lpthread

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY:clean tags

clean:
	$(RM) $(TARGET)


tags:
	ctags -R *

