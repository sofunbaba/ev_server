#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/buffer.h>

#include "server.h"
#include "master_func.h"
#include "client_func.h"
#include "util.h"

struct evbuffer *master_read_buff;
struct evbuffer *master_write_buff;

/*
 * read task from master read buff which cgminers sendto,
 * and transport the task to fpga.
 */
static void *master_read_func(void *arg)
{
    size_t len = 0;
    char *task_str = NULL;
    char task_bin[DEFAULT_TASK_BIN_LEN];
    ev_uint8_t channel = 0, chip = 0;

    log_msg(E_INFO, "Create master read thread.");

    master_read_buff = evbuffer_new();
    evbuffer_enable_locking(master_read_buff, NULL);

    while(1)
    {
        task_str = evbuffer_readln(master_read_buff, &len, EVBUFFER_EOL_ANY);
        if(len > 0)
        {
            log_msg(E_DEBUG, "Master read len:%lu", len);
            log_msg(E_DEBUG, "task:%s", task_str);

            if(len == DEFAULT_TASK_RECV_LEN)
            {

                memset(task_bin, 0, sizeof(task_bin));
                if(hex2bin((unsigned char *)task_bin, task_str, DEFAULT_TASK_BIN_LEN) == false)
                {
                    log_msg(E_DEBUG, "Convert hex to bin error.");
                    goto _END;
                }

                channel     = task_bin[1];
                chip        = task_bin[2];
                task_bin[1] = task_bin[0];

                log_msg(E_DEBUG, "channel:0x%02x, chip:0x%02x", channel, chip);


                evbuffer_add(gl_client_out_buff[channel][chip], &task_bin[1], DEFAULT_TASK_BIN_LEN-DEFAULT_TASK_HEAD_EXPAND);
            }
            else
                log_msg(E_DEBUG, "Invalid task.");

_END:
            free(task_str);
        }
    }

    return NULL;
}

static void *master_write_func(void *arg)
{
    log_msg(E_INFO, "Create master write thread.");

    while(1)
    {
        sleep(1);
    }

    return NULL;
}

void master_thread_init(void *arg)
{
    pthread_t pt;
    int ret = 0;
    struct event_base *base = arg;

    ret = pthread_create(&pt, NULL, master_read_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master read error.");
        event_base_loopexit(base, NULL);
    }

    ret = pthread_create(&pt, NULL, master_write_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master write error.");
        event_base_loopexit(base, NULL);
    }

    log_msg(E_INFO, "Create master thread success.");
}

