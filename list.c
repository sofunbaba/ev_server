#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

void list_head_init(struct list_head *list)
{
    assert(list != NULL);

    list->head   = NULL;
    list->tail   = NULL;
    list->length = 0;

    pthread_mutex_init(&list->lock, NULL);
}

void list_add(struct list_head *list, void *data)
{
    pthread_mutex_lock(&list->lock);

    list_node_t *new = (list_node_t *)malloc(sizeof(list_node_t));

    new->private_data = data;
    new->prev = NULL;
    new->next = NULL;

    if(list->head == NULL)
        list->head = new;
    else
    {
        new->prev = list->tail;
        list->tail->next = new;
    }

    list->tail = new;
    list->length++;

    pthread_mutex_unlock(&list->lock);
}

void list_del(struct list_head *list, list_node_t *node)
{
    pthread_mutex_lock(&list->lock);

    if(node == list->head)
    {
       if(node == list->tail)
       {
           list->head = NULL;
           list->tail = NULL;
       }
       else
       {
           node->next->prev = NULL;
           list->head = node->next;
           node->next = NULL;
       }
   }
   else if(node == list->tail)
   {
       node->prev->next = NULL;
       list->tail = node->prev;
       node->prev = NULL;
   }
   else
   {
       node->prev->next = node->next;
       node->next->prev = node->prev;
       node->next = NULL;
       node->prev = NULL;
   }

   list->length--;
   free(node);

   pthread_mutex_unlock(&list->lock);
}

void list_clear(struct list_head *list)
{
    while(list->head)
        list_del(list,list->head);
}

void print_list(struct list_head *list)
{
    list_node_t *np = NULL;

    printf("list length:%d\r\n", list->length);
    for(np=list->head; np; np=np->next)
        printf("data:0x%p\r\n", np->private_data);
}

int test()
{
    int i=0;
    int *p = NULL;
    struct list_head list = {NULL, NULL, 0};
    list_node_t *np = NULL;

    for(i=0; i<20; i++)
    {
        p = (int *)malloc(sizeof(int));
        *p = i;
        list_add(&list, (void *)p);
    }

    print_list(&list);

    for(np=list.head; np ;np=np->next)
        free(np->private_data);

    list_clear(&list);

    print_list(&list);

    return 0;
}


















