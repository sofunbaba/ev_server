#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "server.h"
#include "master_func.h"
#include "client_func.h"
#include "util.h"
#include "fpga_opt.h"

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
    struct timeval use_time = {0, 0}, now_time={0, 0};

    while(1)
    {
        task_str = evbuffer_readln(master_read_buff, &len, EVBUFFER_EOL_ANY);
        if(len > 0)
        {
            // log_msg(E_DEBUG, "Master read len:%u", len);
            // log_msg(E_DEBUG, "task:%s", task_str);

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

                    if((channel<DEFAULT_CLIENT_CHANNELS) && (chip<DEFAULT_CLIENT_CHIPS))
                    {
                        // log_msg(E_DEBUG, "channel:0x%02x, chip:0x%02x", channel, chip);

                        if(fpga_send_pkg(channel, (unsigned char *)&task_bin[1], DEFAULT_TASK_SEND_LEN) == false)
                            log_msg(E_ERROR, "Send task to fpga error.");
                        else
                        {
                            pthread_mutex_lock(&gl_chip_info.lock);
                            if(gl_chip_info.cinfo[channel][chip])
                            {
                                gettimeofday(&now_time, NULL);
                                evutil_timersub(&now_time, &gl_chip_info.cinfo[channel][chip]->recv_time, &use_time);

                                // log_msg(E_DEBUG, "Master send task to fpga: %ldus", use_time.tv_sec*1000000+use_time.tv_usec);
                            }
                            pthread_mutex_unlock(&gl_chip_info.lock);
                        }
                    }
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
    int len = 0;
    unsigned int channel=0, chip=0;
    char *buff_str = NULL;
    unsigned char buff_bin[DEFAULT_TASK_BIN_LEN];
    struct client_info *cinfo = NULL;
    struct evbuffer *output= NULL;
    char send_buff[200];
    ssize_t ret = 0;

    while(1)
    {
        for(channel=0; channel<DEFAULT_CLIENT_CHANNELS; channel++)
        {
            memset(buff_bin, 0 ,sizeof(buff_bin));

            if(fpga_read_pkg(buff_bin, channel, &len) == true)
            {
                chip = buff_bin[1];
                buff_str = bin2hex(buff_bin, len);

                if(chip < DEFAULT_CLIENT_CHIPS)
                {
                    pthread_mutex_lock(&gl_chip_info.lock);

                    cinfo = gl_chip_info.cinfo[channel][chip];
                    if(cinfo != NULL)
                    {
                        log_msg(E_DEBUG, "Master read fpga channel:0x%x, chip:0x%x", channel, chip);
                        // output = bufferevent_get_output(cinfo->bev);
                        // evbuffer_add_printf(output, "%02x%02x%s\r\n",buff_bin[0], channel, &buff_str[2]);
                        sprintf(send_buff, "%02x%02x%s\r\n", buff_bin[0], channel, &buff_str[2]);
                        send(cinfo->fd, send_buff, len*2+4, 0);
                    }

                    pthread_mutex_unlock(&gl_chip_info.lock);
                }
                else
                    log_msg(E_ERROR, "Master read error chip num(%u).", chip);

                free(buff_str);
            }
        }
    }

    return NULL;
}

bool master_thread_init()
{
    pthread_t pt;
    int ret=0, t=0;

    memset(&gl_chip_info, 0, sizeof(gl_chip_info));
    pthread_mutex_init(&gl_chip_info.lock, 0);

/*
 *     master_read_buff = evbuffer_new();
 *     evbuffer_enable_locking(master_read_buff, NULL);
 *
 *
 *     for(t=0; t<DEFAULT_MASTER_READ_THREAD; t++)
 *     {
 *         ret = pthread_create(&pt, NULL, master_read_func, NULL);
 *         if(ret)
 *         {
 *             log_msg(E_ERROR, "Create thread master read %d error.", t);
 *             return false;
 *         }
 *     }
 */

    ret = pthread_create(&pt, NULL, master_write_func, NULL);
    if(ret)
    {
        log_msg(E_ERROR, "Create thread master write error.");
        return false;
    }

    log_msg(E_INFO, "Create master thread success.");

    return true;
}

