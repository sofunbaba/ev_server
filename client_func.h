#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include <event2/buffer.h>

#define DEFAULT_CLIENT_CHANNELS 6
#define DEFAULT_CLIENT_CHIPS    96

//pkg(53) + channel(1) + \r\n(2)
#define DEFAULT_TASK_LEN 110


extern struct evbuffer *gl_client_out_buff[DEFAULT_CLIENT_CHANNELS][DEFAULT_CLIENT_CHIPS];

void *client_func(void *arg);































#endif

