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
    char *task = NULL;
    char *task_str = NULL, *task_hex = NULL;
    ev_uint8_t channel = 0, chip = 0;

    log_msg(E_INFO, "Create master read thread.");

    master_read_buff = evbuffer_new();
    evbuffer_enable_locking(master_read_buff, NULL);

    while(1)
    {
        task = evbuffer_readln(master_read_buff, &len, EVBUFFER_EOL_ANY);
        if(len > 0)
        {
            log_msg(E_DEBUG, "Master read len:%lu", len);

            if(len == DEFAULT_TASK_LEN)
            {
                channel = task[1];
                chip    = task[2];
                task[1] = task[0];

                task_hex = &task[1];

                log_msg(E_DEBUG, "channel:%d, chip:%d", channel, chip);

                task_str = bin2hex(task_hex, len-1);
                log_msg(E_DEBUG, "task:%s", task_str);
                free(task_str);

                evbuffer_add(gl_client_out_buff[channel][chip], task_hex, len-1);
            }
            free(task);
        }
    }
}

static void *master_write_func(void *arg)
{
    while(1)
    {
        sleep(1);
    }
}

void master_thread_init()
{
    pthread_t pt;
    int ret = 0;

    ret = pthread_create(&pt, NULL, master_read_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master read error.");
        list_event_loopexit();
    }

    ret = pthread_create(&pt, NULL, master_write_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master write error.");
        list_event_loopexit();
    }

    log_msg(E_INFO, "Create master thread success.");
}

