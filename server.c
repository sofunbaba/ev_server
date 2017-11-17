#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event2/thread.h>

#include "server.h"
#include "list.h"
#include "client_func.h"
#include "master_func.h"

/*
 * the global debug level flag.
 */
DEBUG_LEVEL_E debug_level = E_DEBUG;

/*
 * the global event base list
 */
struct list_head gl_event_base;

static void accept_error_cb(struct evconnlistener *listener, void *args)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();

    log_msg(E_ERROR, "Got an error %d (%s) on the listener.", err, evutil_socket_error_to_string(err));

    list_event_loopexit();
}

static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void *args)
{
    ev_int8_t ret = 0;
    pthread_t pt  = 0;
    struct sockaddr_in *sin    = (struct sockaddr_in *)sock;
    evutil_socket_t *client_fd = NULL;

    log_msg(E_DEBUG, "Accept fd:%d. client:%s:%d", fd, inet_ntoa(sin->sin_addr), sin->sin_port);

    client_fd  = (evutil_socket_t *)malloc(sizeof(evutil_socket_t));
    *client_fd = fd;
    ret = pthread_create(&pt, NULL, client_func, client_fd);
    log_msg(E_DEBUG, "Create thread client fd:%d.", *client_fd);
    if(ret != 0)
    {
        free(client_fd);
        log_msg(E_ERROR, "Create pthread error!");
        list_event_loopexit();
    }
}

static void sigint_cb(evutil_socket_t signal, short event, void *args)
{
    log_msg(E_INFO, "Catch the signal (%d).", signal);

    list_event_loopexit();
}

/*
 * notify all the base loop for cleanning up the memory, and free the list.
 */
void list_event_loopexit()
{
    list_node_t *np = NULL;

    for(np=gl_event_base.head; np; np=np->next)
        event_base_loopexit(np->private_data, NULL);

    list_clear(&gl_event_base);

    log_msg(E_DEBUG, "Free all memory and list.");
}

/*
 * alloc a new event_base, and recorded with the global list.
 */
struct event_base *list_event_base_new()
{
    struct event_base *base = NULL;

    base = event_base_new();
    list_add(&gl_event_base, base);

    log_msg(E_DEBUG, "New a event base loop.(cur:%u)",gl_event_base.length);

    return base;
}

void list_event_base_free(struct list_head *list, struct event_base *base)
{
    list_node_t *np = NULL;

    for(np=gl_event_base.head; np; np=np->next)
        if(np->private_data == base)
            list_del(&gl_event_base, np);

    event_base_free(base);

    log_msg(E_DEBUG, "Delete a evnet base.(remain:%d)", gl_event_base.length);
}

int main(int argc, char *argv[])
{
    struct event_base     *base      = NULL;
    struct event          *e_sigint  = NULL;
    struct event          *e_sigusr1 = NULL;
    struct evconnlistener *listener  = NULL;
    struct sockaddr_in    sin;

    ev_uint16_t port = DEFAULT_SERVER_PORT;

    /*
     * init the list
     */
    list_head_init(&gl_event_base);

    if(argc == 2)
        port = atoi(argv[1]);

    if(port < 1000 || port > EV_UINT16_MAX)
    {
        log_msg(E_ERROR, "Invalid port!");
        return 1;
    }

    log_msg(E_INFO, "The server port: %d", port);

    master_thread_init();

    evthread_use_pthreads();
    base = list_event_base_new();

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(0);     //all address

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *)&sin, sizeof(sin));
    if(!listener)
    {
        log_msg(E_ERROR, "Couldn't create listner.");
        return 1;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);

    /*
     * catch the SIGINT signal to clean memory
     */
    e_sigint = event_new(base, SIGINT, EV_SIGNAL, sigint_cb, NULL);
    event_add(e_sigint, NULL);

    /*
     * add the SIGUSR1 signal for kill cmd
     */
    e_sigusr1 = event_new(base, SIGUSR1, EV_SIGNAL, sigint_cb, NULL);
    event_add(e_sigusr1, NULL);

    log_msg(E_INFO, "Server is running...");

    event_base_dispatch(base);

    log_msg(E_INFO, "Server is shutting down...");

    evconnlistener_free(listener);
    event_free(e_sigint);
    event_free(e_sigusr1);
    event_base_free(base);
    libevent_global_shutdown();

    return 0;
}

