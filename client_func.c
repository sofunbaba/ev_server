#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "server.h"
#include "client_func.h"
#include "master_func.h"
#include "util.h"

/*
 * the global client info list.
 */
struct list_head gl_client_info;

/*
 * storge all the client output buffer
 */
struct evbuffer *gl_client_out_buff[DEFAULT_CLIENT_CHANNELS][DEFAULT_CLIENT_CHIPS];

/*
 * read task from the client,
 * and send the task to the master read buff.
 *
 */
void client_read_cb(struct bufferevent *bev, void *arg)
{
    char *task_str = NULL, task_bin[DEFAULT_TASK_BIN_LEN];
    size_t len = 0;
    ev_uint8_t channel = 0, chip = 0;
    struct evbuffer *input  = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);
    evutil_socket_t fd      = bufferevent_getfd(bev);

    /*
     * read task that end with '\r\n' or '\r' or '\n'
     */
    task_str = evbuffer_readln(input, &len, EVBUFFER_EOL_ANY);
    if(len > 0)
    {
        log_msg(E_DEBUG, "Client:%d got a message, len:%lu", fd, len);
        log_msg(E_DEBUG, "recv:%s", task_str);

        if(len == DEFAULT_TASK_RECV_LEN)
        {
            memset(task_bin, 0, sizeof(task_bin));
            if(hex2bin((unsigned char *)task_bin, task_str, DEFAULT_TASK_HEAD_LEN) == false)
            {
                log_msg(E_DEBUG, "Convert hex to bin error.");
                goto _END;
            }

            if(task_bin[0] == DEFAULT_TASK_START)
            {
                channel = task_bin[1];
                chip    = task_bin[2];

                gl_client_out_buff[channel][chip] = output;

                log_msg(E_DEBUG, "channel:0x%02x, chip:0x%02x", channel, chip);

                evbuffer_add_printf(master_read_buff, "%s\r\n", task_str);
            }
            else
                log_msg(E_DEBUG, "Invalid task head.");
        }
        else
            log_msg(E_DEBUG, "Invalid task.");

_END:
        free(task_str);
   }
}

void client_error_cb(struct bufferevent *bev, short what, void *arg)
{
    struct client_info *cinfo = arg;
    int err = EVUTIL_SOCKET_ERROR();

    if(what & BEV_EVENT_EOF)
        log_msg(E_DEBUG, "Client:%d exit.", cinfo->fd);
    else if(what & BEV_EVENT_ERROR)
        log_msg(E_ERROR, "Client:%d got a error(%s).", cinfo->fd, evutil_socket_error_to_string(err));

    event_base_loopexit(cinfo->base, NULL);
}

void list_client_info_free(struct client_info *cinfo)
{
    list_node_t *np = NULL;

    pthread_mutex_lock(&gl_client_info.lock);
    for(np=gl_client_info.head; np; np=np->next)
        if(np->private_data == cinfo)
            list_del(&gl_client_info, np);
    pthread_mutex_unlock(&gl_client_info.lock);

    log_msg(E_DEBUG, "Delete a client.(remain:%d)", gl_client_info.length);
}

void list_client_info_add(struct client_info *cinfo)
{
    pthread_mutex_lock(&gl_client_info.lock);

    list_add(&gl_client_info, cinfo);

    pthread_mutex_unlock(&gl_client_info.lock);
}

void *client_func(void *arg)
{
    struct client_info *cinfo = arg;

    log_msg(E_DEBUG, "Create thread fd:%d", cinfo->fd);

    cinfo->base = event_base_new();

    cinfo->bev = bufferevent_socket_new(cinfo->base, cinfo->fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(cinfo->bev, client_read_cb, NULL, client_error_cb, cinfo);
    bufferevent_enable(cinfo->bev, EV_READ);

    /*
     * fill the client info.
     */
    memset(cinfo->client_name, 0, sizeof(cinfo->client_name));
    cinfo->uptime = 0;
    event_base_dispatch(cinfo->base);

    log_msg(E_DEBUG, "Thread client fd:%d exit!", cinfo->fd);

    close(cinfo->fd);
    bufferevent_free(cinfo->bev);
    event_base_free(cinfo->base);
    list_client_info_free(cinfo);

    return NULL;
}
