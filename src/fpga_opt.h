#ifndef _FPGA_OPT_H_
#define _FPGA_OPT_H_

#include <pthread.h>
#include "util.h"

#define FPGA_READ_LEN 4
#define FPGA_OPT_RD   0
#define FPGA_OPT_WR   1
#define FPGA_MAP_SIZE 0x100000
#define FPGA_MAP_MASK (FPGA_MAP_SIZE-1)

#define FPGA_CHANNEL_WR_REG     (7<<2)
#define FPGA_CHANNEL_RD_REG     (6<<2)
#define FPGA_TASK_LEN_REG       (0x200<<2)
#define FPGA_TASK_REG(x)        ((0x201+x)<<2)
#define FPGA_TASK_END_REG       (1<<2)
#define FPGA_TASK_END_FLAG      (1<<7)
#define FPGA_HAS_DATA(x)        (x&(1<<7))
#define FPGA_GET_DATA_LEN(x)    (x&0x3f)
#define FPGA_READ_REG_OFFSET(x) ((x+0x50)<<2)

#define g_fpga_base_map  (gl_fpga_base.map)
#define g_fpga_base_lock (gl_fpga_base.lock)
#define g_fpga_base_fd (gl_fpga_base.mem_fd)

struct fpga_base {
    void *map;
    int mem_fd;
    pthread_mutex_t lock;
};


bool fpga_init();
void fpga_map_destroy();
bool fpga_read_pkg(unsigned char *buf, unsigned char channel, int *len);
int fpga_send_pkg(unsigned char channel,unsigned char *buf,int len);































#endif

