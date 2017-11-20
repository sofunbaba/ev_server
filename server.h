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
 * default update the client uptimes one time pre second.
 */
#define DEFAULT_UPDATE_TIME {1, 0}

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
                                            printf("%10s[%s: %s: %d] ", "--> ", __FILE__,__func__, __LINE__); \
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
    E_ALL,
}DEBUG_LEVEL_E;


/*
 * the global debug level flag.
 */
extern DEBUG_LEVEL_E debug_level;

/*
 * notify all the client for cleanning up the memory, and free the list.
 */
void list_client_loopexit();








#endif

