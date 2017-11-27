#ifndef _MASTER_FUNC_H_
#define _MASTER_FUNC_H_

#include "master_func.h"
#include "util.h"

#define DEFAULT_MASTER_READ_THREAD 1


extern struct evbuffer *master_read_buff;
extern struct evbuffer *master_write_buff;

bool master_thread_init();









































#endif

