#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "fpga_opt.h"
#include "server.h"
#include "client_func.h"

struct fpga_base gl_fpga_base;

bool fpga_init()
{

    if((g_fpga_base_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
    {
        log_msg(E_ERROR, "/dev/mem opened error.");
        return false;
    }

    g_fpga_base_map = mmap(0, FPGA_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fpga_base_fd, 0x40000000 & ~FPGA_MAP_MASK);
    if(g_fpga_base_map == (void *) -1)
    {
        log_msg(E_ERROR, "Memory mapped error. At address %p.", g_fpga_base_map);
        return false;
    }

    pthread_mutex_init(&g_fpga_base_lock, NULL);

    return true;
}

void fpga_map_destroy()
{
    close(g_fpga_base_fd);
    munmap(g_fpga_base_map, FPGA_MAP_SIZE);
}

static unsigned long fpga_mem_opt(int opt, unsigned long offset, int access_type, unsigned long value)
{
    void *virt_addr = NULL;
    unsigned long read_result = 0;

    virt_addr = g_fpga_base_map + (offset & 0xFFFFF);

    if(opt)
    {
        switch(access_type) {
            case 1:
                *((unsigned char *) virt_addr) = value;
                read_result = *((unsigned char *) virt_addr);
                break;
            case 2:
                *((unsigned short *) virt_addr) = value;
                read_result = *((unsigned short *) virt_addr);
                break;
            case 4:
                *((unsigned long *) virt_addr) = value;
                read_result = *((unsigned long *) virt_addr);
                break;
        }
    }
    else
    {
        switch(access_type) {
            case 1:
                read_result = *((unsigned char *) virt_addr);
                break;
            case 2:
                read_result = *((unsigned short *) virt_addr);
                break;
            case 4:
                read_result = *((unsigned long *) virt_addr);
                break;
            default:
                log_msg(E_ERROR, "Illegal data type %d.", access_type);
        }
    }

    return read_result;
}

int fpga_send_pkg(unsigned char channel,uint8_t *buf,int len)
{
    int i=0, times=100, rdlen=0;

    if(channel >= DEFAULT_CLIENT_CHANNELS)
    {
        log_msg(E_ERROR, "FPGA send task channel error.");
        return false;
    }

    /*
     * check if fpga is ready.
     */
    while(times--)
    {
        rdlen = fpga_mem_opt(FPGA_OPT_RD, FPGA_TASK_END_REG, FPGA_READ_LEN, 0);
        if(FPGA_HAS_DATA(rdlen))
            usleep(100);
        else
            break;
    }

    if(times <= 0)
    {
        log_msg(E_ERROR, "FPGA is too busy.");
        return false;
    }

    pthread_mutex_lock(&g_fpga_base_lock);

    /*
     * select the channel.
     */
    fpga_mem_opt(FPGA_OPT_WR, FPGA_CHANNEL_WR_REG, FPGA_READ_LEN, channel);

    if(len > 64)
    {
        log_msg(E_ERROR, "FPGA send task len error.");
        pthread_mutex_unlock(&g_fpga_base_lock);
        return false;
    }

    /*
     * send the total task buff len.
     */
    fpga_mem_opt(FPGA_OPT_WR, FPGA_TASK_LEN_REG, FPGA_READ_LEN, len);

    /*
     * send the task buff.
     */
    for(i=0; i<len; i++)
        fpga_mem_opt(FPGA_OPT_WR, FPGA_TASK_REG(i), FPGA_READ_LEN, buf[i]);

    /*
     * send task end.
     */
    fpga_mem_opt(FPGA_OPT_WR, FPGA_TASK_END_REG, FPGA_READ_LEN, FPGA_TASK_END_FLAG);

    pthread_mutex_unlock(&g_fpga_base_lock);

    return true;
}

/*
 * read result from fpga.
 * return value:
 *     false: no result,
 *     ture:  has result.
 */
bool fpga_read_pkg(unsigned char *buf, unsigned char channel, int *len)
{
    int i = 0;
    unsigned long offset = 0, rdlen = 0;

    if(channel >= DEFAULT_CLIENT_CHANNELS)
    {
        log_msg(E_ERROR, "FPGA send task channel error.");
        return false;
    }

    pthread_mutex_lock(&g_fpga_base_lock);

    fpga_mem_opt(FPGA_OPT_WR, FPGA_CHANNEL_RD_REG, FPGA_READ_LEN, channel);

    if(channel >= 4)
        offset = 0x176 + channel - 4;
    else
        offset = 2 + channel;

    rdlen = fpga_mem_opt(FPGA_OPT_RD, offset<<2, FPGA_READ_LEN, 0);

    if(FPGA_HAS_DATA(rdlen))
    {
        for(i=0; i<FPGA_GET_DATA_LEN(rdlen); i++)
            buf[i] = (unsigned char)fpga_mem_opt(FPGA_OPT_RD, FPGA_READ_REG_OFFSET(i), FPGA_READ_LEN, 0);

        *len = FPGA_GET_DATA_LEN(rdlen);

        pthread_mutex_unlock(&g_fpga_base_lock);
        return true;
    }

    pthread_mutex_unlock(&g_fpga_base_lock);

    return false;
}

