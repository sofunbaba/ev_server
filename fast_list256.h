#ifndef _FAST_LIST_256_H_
#define _FAST_LIST_256_H_

#define LIST256_FULL 0xfffful

#define list256_length(list_p) listp_p->length

struct list256_base {
    unsigned short length;
    unsigned short group;
    unsigned short bitmaps[16];
};

unsigned short list256_add_pos(struct list256_base *base);
unsigned short list256_del_pos(struct list256_base *base, unsigned short pos);









#endif

