#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "server.h"
#include "client_func.h"
#include "master_func.h"
#include "util.h"

#define hex_to_str_len(str_len) (str_len*2)

/*
 * storge all the client output buffer
 */
struct evbuffer *gl_client_out_buff[DEFAULT_CLIENT_CHANNELS][DEFAULT_CLIENT_CHIPS];

/*
 * read task from the client,
 * and transport the task to the master read queue buff.
 *
 */
void client_read_cb(struct bufferevent *bev, void *arg)
{
    ev_uint8_t ret = 0;
    char *task = NULL;
    size_t len = 0;
    char task_hex[DEFAULT_TASK_LEN]; //has to add \r\n or \n or \0 to the tail
    ev_uint8_t channel=0, chip=0;
    struct evbuffer *input  = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);
    evutil_socket_t fd      = bufferevent_getfd(bev);


    task = evbuffer_readln(input, &len, EVBUFFER_EOL_ANY);
    if(len > 0)
    {
        if(len == (DEFAULT_TASK_LEN-2))
        {
            log_msg(E_DEBUG, "Client:%d got a message:%s", fd, task);

            memset(task_hex, 0, sizeof(task_hex));
            ret = hex2bin(task_hex, task, len/2);
            if(!ret)
            {
                log_msg(E_ERROR, "convert str to hex error.");
                return;
            }

            if(task_hex[0] == 0x5a)
            {
                channel = task_hex[1];
                chip    = task_hex[2];
                gl_client_out_buff[channel][chip] = output;

                task_hex[1] = task_hex[0];

                task_hex[len/2] = '\n';

                evbuffer_add(master_read_buff, &task_hex[1], len/2+1);
            }
        }
        free(task);
    }
}

void client_error_cb(struct bufferevent *bev, short what, void *arg)
{
    struct event_base *base = bufferevent_get_base(bev);
    evutil_socket_t fd = bufferevent_getfd(bev);

    if(what & BEV_EVENT_EOF)
        log_msg(E_DEBUG, "client:%d exit.", fd);
    else if(what & BEV_EVENT_ERROR)
        log_msg(E_ERROR, "client:%d got a error.", fd);

    event_base_loopexit(base, NULL);
}

void *client_func(void *arg)
{
    struct bufferevent *bev = NULL;
    struct event_base *base = NULL;

    evutil_socket_t fd = *(evutil_socket_t *)arg;

    log_msg(E_DEBUG, "Create thread fd:%d", fd);

    base = list_event_base_new();

    bev = bufferevent_socket_new(base, fd, LEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, client_read_cb, NULL, client_error_cb, NULL);
    bufferevent_enable(bev, EV_READ);

    event_base_dispatch(base);

    log_msg(E_DEBUG, "thread client fd:%d exit!", fd);

    free(arg);
    bufferevent_free(bev);
    list_event_base_free(&gl_event_base, base);
}

