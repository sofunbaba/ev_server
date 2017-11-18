#include "cmd_process.h"
#include "server.h"
#include "client_func.h"
#include "util.h"

#include <event2/bufferevent.h>
#include <event2/buffer.h>

client_cmd_handle gl_client_cmd[DEFAULT_CLIENT_CMD_MAX];

void process_cmd(char *pkg, struct client_info *cinfo)
{
    struct evbuffer *output = bufferevent_get_output(cinfo->bev);

    ev_uint32_t cmd     = 0;
    ev_uint8_t arg[100];

    pkg_parse_data(pkg, &cmd, PKG_FIELD_CMD);
    pkg_parse_data(pkg, arg, PKG_FIELD_ARGS);

    log_msg(E_DEBUG, "execute the cmd: 0x%08x", cmd);

    cmd = remote_cmd(cmd);
    log_msg(E_DEBUG, "remote cmd: 0x%08x", cmd);

    cmd = local_cmd(cmd);
    log_msg(E_DEBUG, "local cmd: 0x%08x", cmd);

    evbuffer_add_printf(output, "You have uptime: %lu\r\n", cinfo->uptime);
}

void client_cmd_init()
{
}
































