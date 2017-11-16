#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "server.h"


/*
 * void client_read_cb(struct bufferevent *bev, void *arg)
 * {
 *     struct evbuffer *input = bufferevent_get_input(bev);
 *     struct evbuffer *output = bufferevent_get_output(bev);
 *     evutil_socket_t fd = bufferevent_getfd(bev);
 *
 *     log_msg(E_DEBUG, "client:%d got a message.", fd);
 *
 *     evbuffer_add_buffer(output, input);
 * }
 */

void client_read_cb(evutil_socket_t fd, short what, void *arg)
{
    int len=0;
    char buff[100];
    struct event_base *base = arg;

    len = read(fd, buff, sizeof(buff));
    if(len == 0)
    {
        log_msg(E_DEBUG, "Client:%d exit.", fd);
        event_base_loopexit(base, NULL);
    }
    else
    {
        printf("%d.recv:%s\r\n", fd, buff);
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
    struct event *e_client= NULL;

    evutil_socket_t fd = *(evutil_socket_t *)arg;

    log_msg(E_DEBUG, "Create thread fd:%d", fd);

    base = list_event_base_new();

    e_client = event_new(base, fd, EV_READ|EV_PERSIST, client_read_cb, base);
    event_add(e_client, NULL);

    // bev = bufferevent_socket_new(base, fd, LEV_OPT_CLOSE_ON_FREE);
    // bufferevent_setcb(bev, client_read_cb, NULL, client_error_cb, NULL);
    // bufferevent_enable(bev, EV_READ);

    event_base_dispatch(base);
    free(arg);

    log_msg(E_DEBUG, "thread client fd:%d exit!", fd);
    // bufferevent_free(bev);
    list_event_base_free(&gl_event_base, base);
}





















