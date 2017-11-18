#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include <event2/util.h>
#include "list.h"

/*
 * Transport every field as Byte
 * eg: "1A2BB2A101020304001E0000001000"
 *
 * ----------------------------------------------------------------------------
 *
 * request  pkgs = head(4) + session(4) + pkglen(2) + cmd(4) + opt(1) + args(n) + "\r\n"
 *
 * response pkgs = head(4) + session(4) + pkglen(2) + result(n) + "\r\n"
 *
 * ----------------------------------------------------------------------------
 *
 * head:             0x1a2bb2a1
 * request session:  it could be a random num.
 * response session: must be request session + 1.
 * pkglen:           total pkgs length but "\r\n".
 * cmd:              function that what to execute.
 * opt:              read or write operate.
 * args:             args of the cmd.
 * result:           the request return data.
 *
 * ----------------------------------------------------------------------------
 */

#define DEFAULT_CLIENT_CMD_MAX  0xff
#define DEFAULT_CLIENT_CMD_MASK 0xabcddcbaul

#define DEFAULT_CMD_HEAD         0x1a2bb2a1ul

#define remote_cmd(cmd) (DEFAULT_CLIENT_CMD_MASK^cmd)
#define local_cmd remote_cmd
#define install_client_cmd(cmd, func) do{gl_client_cmd[local_cmd[cmd]] = func;}while(0)

typedef enum {
    PKG_FIELD_HEAD,
    PKG_FIELD_SESSION,
    PKG_FIELD_PKGLEN,
    PKG_FIELD_CMD,
    PKG_FIELD_OPT,
    PKG_FIELD_ARGS,
    PKG_FIELD_MAX,
}pkg_field_type_t;

struct pkg_field {
    ev_uint8_t position;
    ev_int8_t  size;
};

struct client_info {
    pthread_t pt;
    evutil_socket_t fd;
    char client_name[100];
    ev_uint64_t uptime;
    struct bufferevent *bev;
    struct sockaddr_in sin;
    struct event_base *base;
};

typedef ev_uint8_t (*client_cmd_handle)(struct client_info *, ev_uint8_t, void *, ev_uint32_t);



/*
 * the global client info list.
 */
extern struct list_head gl_client_info;
extern struct pkg_field pkg_filed_data[PKG_FIELD_MAX];


void *client_func(void *arg);
void list_client_info_free(struct client_info *cinfo);
void list_client_info_add(struct client_info *cinfo);


/*
 * parse pkg filed.
 */
ev_uint16_t pkg_parse_data(char *pkg, void *field, pkg_field_type_t type);































#endif

