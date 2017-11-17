#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "server.h"
#include "client_func.h"
#include "util.h"

/*
 * read task from the client.
 */
void client_read_cb(struct bufferevent *bev, void *arg)
{
    ev_uint8_t ret = 0;
    char *task = NULL, *task_p = NULL;
    size_t len = 0;
    struct evbuffer *input  = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);
    evutil_socket_t fd      = bufferevent_getfd(bev);

    /*
     * read task that end with '\r\n' or '\r' or '\n'
     */
    task = evbuffer_readln(input, &len, EVBUFFER_EOL_ANY);
    if(len > 0)
    {
        log_msg(E_DEBUG, "Client:%d got a message, len:%lu", fd, len);

        if(len == DEFAULT_TASK_LEN)
        {
            if(task[0] == DEFAULT_TASK_HEAD)
            {
                task_p = bin2hex(task, len);
                log_msg(E_DEBUG, "recv:%s", task_p);
                free(task_p);
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
        log_msg(E_DEBUG, "Client:%d exit.", fd);
    else if(what & BEV_EVENT_ERROR)
        log_msg(E_ERROR, "Client:%d got a error.", fd);

    bufferevent_free(bev);
    event_base_loopexit(base, NULL);
}

void *client_func(void *arg)
{
    struct bufferevent *bev = NULL;
    struct event_base *base = NULL;

    evutil_socket_t fd = *(evutil_socket_t *)arg;

    log_msg(E_DEBUG, "Create thread fd:%d", fd);

    base = list_event_base_new();

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, client_read_cb, NULL, client_error_cb, NULL);
    bufferevent_enable(bev, EV_READ);

    event_base_dispatch(base);

    log_msg(E_DEBUG, "Thread client fd:%d exit!", fd);

    free(arg);
    list_event_base_free(&gl_event_base, base);
}

