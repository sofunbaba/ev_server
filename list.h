#ifndef _LIST_H_
#define _LIST_H_

typedef struct _list_node{
    void *private_data;
    struct _list_node *prev;
    struct _list_node *next;
}list_node_t;

struct list_head {
    list_node_t *head;
    list_node_t *tail;
    unsigned int length;
};

void list_add(struct list_head *list, void *data);
void list_del(struct list_head *list, list_node_t *node);
void list_clear(struct list_head *list);







































#endif

