#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include <event2/buffer.h>
#include <event2/util.h>
#include "list.h"


#define DEFAULT_CLIENT_CHANNELS 6
#define DEFAULT_CLIENT_CHIPS    96

#define DEFAULT_TASK_RECV_LEN 108
#define DEFAULT_TASK_BIN_LEN (DEFAULT_TASK_RECV_LEN/2)
#define DEFAULT_TASK_HEAD_EXPAND 1
#define DEFAULT_TASK_HEAD_LEN (5+DEFAULT_TASK_HEAD_EXPAND)
#define DEFAULT_TASK_START 0x5a

struct client_info {
    pthread_t pt;
    evutil_socket_t fd;
    char client_name[100];
    ev_uint64_t uptime;
    struct bufferevent *bev;
    struct sockaddr_in sin;
    struct event_base *base;
};

/*
 * the global client info list.
 */
extern struct list_head gl_client_info;

extern struct evbuffer *gl_client_out_buff[DEFAULT_CLIENT_CHANNELS][DEFAULT_CLIENT_CHIPS];

void *client_func(void *arg);
void list_client_info_free(struct client_info *cinfo);
void list_client_info_add(struct client_info *cinfo);
































#endif

