#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "server.h"
#include "util.h"
#include "client_func.h"
#include "cmd_process.h"

/*
 * the global client info list.
 */
struct list_head gl_client_info;

static bool verify_pkg(char *pkg, size_t len)
{
    bool ret = true;
    ev_uint8_t opt = 0, args[100];
    ev_uint32_t head, session, cmd, arglen;
    ev_uint16_t pkglen;

    assert(pkg != NULL);

    pkg_parse_data(pkg, &head, PKG_FIELD_HEAD);
    ret &= (head==DEFAULT_CMD_HEAD);

    pkg_parse_data(pkg, &session, PKG_FIELD_SESSION);

    pkg_parse_data(pkg, &pkglen, PKG_FIELD_PKGLEN);
    ret &= (pkglen==len);

    pkg_parse_data(pkg, &cmd, PKG_FIELD_CMD);
    pkg_parse_data(pkg, &opt, PKG_FIELD_OPT);
    arglen = pkg_parse_data(pkg, args, PKG_FIELD_ARGS);

    log_msg(E_DEBUG, "Head: 0x%08x, Session: 0x%08x, pkglen: 0x%04x, cmd: 0x%08x, opt: 0x%02x, arglen: %d, args: %s", head, session, pkglen, cmd, opt, arglen, args);

    log_msg(E_DEBUG, "ret: %d", ret);

    return ret;
}

/*
 * read cmd from the client.
 */
void client_read_cb(struct bufferevent *bev, void *arg)
{
    char *pkg = NULL;
    size_t len = 0;
    struct client_info *cinfo = arg;
    struct evbuffer *input    = bufferevent_get_input(bev);

    /*
     * read cmd that end with '\r\n' or '\r' or '\n'
     */
    pkg = evbuffer_readln(input, &len, EVBUFFER_EOL_ANY);
    if(len > 0)
    {
        log_msg(E_DEBUG, "Client:%d got a message, len:%lu", cinfo->fd, len);

        if(verify_pkg(pkg, len))
        {
            log_msg(E_ALL, "recv:%s", pkg);

            process_cmd(pkg, cinfo);
        }

        free(pkg);
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

    /*
     * add the client info to the list.
     */
    list_client_info_add(cinfo);

    event_base_dispatch(cinfo->base);

    log_msg(E_DEBUG, "Thread client fd:%d exit!", cinfo->fd);

    close(cinfo->fd);
    bufferevent_free(cinfo->bev);
    event_base_free(cinfo->base);
    list_client_info_free(cinfo);

    return NULL;
}

struct pkg_field pkg_field_data[PKG_FIELD_MAX] = {
    {0, 4},
    {8, 4},
    {16, 2},
    {20, 4},
    {28, 1},
    {30, -1},
};

ev_uint16_t pkg_get_arglen(char *pkg)
{
    ev_uint16_t pkglen = 0;

    pkg_parse_data(pkg, &pkglen, PKG_FIELD_PKGLEN);
    pkglen -= pkg_field_data[PKG_FIELD_ARGS].position;

    return (pkglen<=0) ? 0 : pkglen;
}

ev_uint16_t pkg_parse_data(char *pkg, void *field, pkg_field_type_t type)
{
    ev_uint32_t *u32p = field;
    ev_uint16_t *u16p = field;

    if(field != NULL)
    {
        if(pkg_field_data[type].size == -1)
        {
            ev_uint16_t arglen = pkg_get_arglen(pkg);
            if(arglen != 0)
                strncpy(field, pkg+pkg_field_data[type].position, arglen);
            *(ev_uint8_t *)(field+arglen) = '\0';

            return arglen;

        }
        else
        {
            hex2bin((ev_uint8_t *)field, pkg+pkg_field_data[type].position, pkg_field_data[type].size);
            if(pkg_field_data[type].size == sizeof(int))
                *u32p = ntohl(*u32p);
            else if(pkg_field_data[type].size == sizeof(short))
                *u16p = ntohs(*u16p);
            else
            {
            }

        }
    }
        return 0;

    return pkg_field_data[type].size;
}


