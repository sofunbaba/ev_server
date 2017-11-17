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

static void *master_read_func(void *arg)
{
    size_t len = 0;
    char *task = NULL;
    char *task_str = NULL;

    log_msg(E_DEBUG, "Create master read thread.");

    master_read_buff = evbuffer_new();
    evbuffer_enable_locking(master_read_buff, NULL);

    while(1)
    {
        task = evbuffer_readln(master_read_buff, &len, EVBUFFER_EOL_ANY);
        if(len > 0)
        {
            printf("master read len:%lu\r\n", len);
            if(len == (DEFAULT_TASK_LEN/2-2))
            {
                task_str = bin2hex(task, len);

                printf("%s\r\n", task_str);

                free(task_str);
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
    pthread_t pt[2];
    int ret = 0;

    ret = pthread_create(&pt[0], NULL, master_read_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master read error.");
        list_event_loopexit();
    }

    ret = pthread_create(&pt[1], NULL, master_write_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master write error.");
        list_event_loopexit();
    }

    log_msg(E_DEBUG, "Create master thread success.");
}

