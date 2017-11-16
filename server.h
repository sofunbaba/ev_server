#ifndef _SERVER_H_
#define _SERVER_H_

#include <event2/event.h>
#include <event2/util.h>
#include "event2/listener.h"

/*
 * default server port. could pass the args to modify
 */
#define DEFAULT_SERVER_PORT 12345

/*
 * print the message for defferent log level
 */
#define log_msg(level, fmt, ...) do{ \
                                    if(level <= debug_level) \
                                    {\
                                        if(level == E_ERROR) \
                                            printf("\033[1m\033[31;40m"); \
                                        printf("[%s] "fmt, #level, ##__VA_ARGS__); \
                                        if(level == E_DEBUG) \
                                            printf("%10s[%s:%s:%d] ", "-->", __FILE__,__func__, __LINE__); \
                                        printf("\033[0m\r\n"); \
                                    } \
                                }while(0)

/*
 * debug level for different message
 */
typedef enum {
    E_ERROR,
    E_INFO,
    E_DEBUG,
}DEBUG_LEVEL_E;


/*
 * the global event base list
 */
extern struct list_head gl_event_base;

/*
 * the global debug level flag.
 */
extern DEBUG_LEVEL_E debug_level;

/*
 * alloc a new event_base, and recorded with the global list.
 */
struct event_base *list_event_base_new();

/*
 * notify all the base loop for cleanning up the memory, and free the list.
 */
void list_event_loopexit();

/*
 * delete a event_base from the global list
 */
void list_event_base_free(struct list_head *list, struct event_base *base);











#endif

