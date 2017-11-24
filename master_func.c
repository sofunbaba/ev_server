#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/bufferevent.h>
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
    struct client_info *cinfo = NULL;
    struct evbuffer *output = NULL;

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
                    log_msg(E_DEBUG, "Convert hex to bin error.");
                else
                {
                    channel     = task_bin[1];
                    chip        = task_bin[2];
                    task_bin[1] = task_bin[0];

                    log_msg(E_DEBUG, "channel:0x%02x, chip:0x%02x", channel, chip);

                    pthread_mutex_lock(&gl_chip_info.lock);

                    cinfo = gl_chip_info.cinfo[channel][chip];
                    if(cinfo != NULL)
                    {
                        output = bufferevent_get_output(cinfo->bev);
                        evbuffer_add(output, &task_bin[1], DEFAULT_TASK_SEND_LEN);
                    }
                    else
                        log_msg(E_ERROR, "Client had exited, clear the invalid task.");

                    pthread_mutex_unlock(&gl_chip_info.lock);
                }
            }
            else
                log_msg(E_DEBUG, "Invalid task.");

            free(task_str);
        }
    }

    return NULL;
}

static void *master_write_func(void *arg)
{
    while(1)
    {
        sleep(1);
    }

    return NULL;
}

bool master_thread_init()
{
    pthread_t pt;
    int ret=0, t=0;

    memset(&gl_chip_info, 0, sizeof(gl_chip_info));
    pthread_mutex_init(&gl_chip_info.lock, 0);

    master_read_buff = evbuffer_new();
    evbuffer_enable_locking(master_read_buff, NULL);


    for(t=0; t<DEFAULT_MASTER_READ_THREAD; t++)
    {
        ret = pthread_create(&pt, NULL, master_read_func, NULL);
        if(ret)
        {
            log_msg(E_ERROR, "Create thread master read %d error.", t);
            return false;
        }
    }

    ret = pthread_create(&pt, NULL, master_write_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master write error.");
        return false;
    }

    log_msg(E_INFO, "Create master thread success.");

    return true;
}
