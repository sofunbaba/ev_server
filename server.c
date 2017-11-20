#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
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

static void accept_error_cb(struct evconnlistener *listener, void *args)
{
    int err = EVUTIL_SOCKET_ERROR();

    log_msg(E_ERROR, "Got an error %d (%s) on the listener.", err, evutil_socket_error_to_string(err));

    list_client_loopexit();
}

static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void *args)
{
    ev_int8_t ret = 0;
    struct client_info *cinfo = NULL;


    cinfo = (struct client_info *)malloc(sizeof(struct client_info));
    assert(cinfo != NULL);

    cinfo->fd = fd;
    memcpy((ev_uint8_t *)&cinfo->sin, (ev_uint8_t *)sock, sizeof(struct sockaddr_in));

    ret = pthread_create(&cinfo->pt, NULL, client_func, (void *)cinfo);

    log_msg(E_DEBUG, "Create thread client fd:%d.", cinfo->fd);
    if(ret != 0)
    {
        free(cinfo);
        log_msg(E_ERROR, "Create pthread error!");
    }
    else
    {

        /*
         * add the client info to the list.
         */
        list_client_info_add(cinfo);
    }

    log_msg(E_DEBUG, "Accept fd:%d. client:%s:%d", cinfo->fd, inet_ntoa(cinfo->sin.sin_addr), cinfo->sin.sin_port);
}

static void sigint_cb(evutil_socket_t signal, short event, void *args)
{
    struct event_base *main_base = args;

    log_msg(E_INFO, "Catch the signal (%d).", signal);

    /*
     * main base loop exit.
     */
    event_base_loopexit(main_base, NULL);
}

static void update_cb(evutil_socket_t signal, short event, void *args)
{
    list_node_t        *np    = NULL;
    struct client_info *cinfo = NULL;

    for(np=gl_client_info.head; np; np=np->next)
    {
        cinfo = np->private_data;
        cinfo->uptime++;
    }
}

/*
 * notify all the base loop for cleanning up the memory, and free the list.
 */
void list_client_loopexit()
{
    list_node_t        *np    = NULL;
    struct client_info *cinfo = NULL;

    pthread_mutex_lock(&gl_client_info.lock);
    for(np=gl_client_info.head; np; np=np->next)
    {
        cinfo = np->private_data;
        event_base_loopexit(cinfo->base, NULL);
    }
    pthread_mutex_unlock(&gl_client_info.lock);

    log_msg(E_DEBUG, "Notify all the base loop exit.");
}

int main(int argc, char *argv[])
{
    struct event_base     *base      = NULL;
    struct event          *e_sigint  = NULL;
    struct event          *e_sigusr1 = NULL;
    struct event          *e_update  = NULL;
    struct client_info    *cinfo     = NULL;
    struct evconnlistener *listener  = NULL;
    struct sockaddr_in    sin;

    struct timeval tv   = DEFAULT_UPDATE_TIME;
    ev_uint16_t    port = DEFAULT_SERVER_PORT;

    /*
     * init the list
     */
    list_head_init(&gl_client_info);

    if(argc == 2)
        port = atoi(argv[1]);

    if(port < 1000 || port > EV_UINT16_MAX)
    {
        log_msg(E_ERROR, "Invalid port!");
        return 1;
    }

    log_msg(E_INFO, "The server port: %d", port);

    evthread_use_pthreads();
    base = event_base_new();

    /*
     * init the master rx and tx thread.
     */
    master_thread_init(base);

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
    e_sigint = event_new(base, SIGINT, EV_SIGNAL, sigint_cb, base);
    event_add(e_sigint, NULL);

    /*
     * add the SIGUSR1 signal for kill cmd
     */
    e_sigusr1 = event_new(base, SIGUSR1, EV_SIGNAL, sigint_cb, base);
    event_add(e_sigusr1, NULL);

    /*
     * update the client uptime pre second.
     */
    e_update = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, update_cb, NULL);
    event_add(e_update, &tv);

    log_msg(E_INFO, "Server is running...");

    event_base_dispatch(base);


    evconnlistener_free(listener);
    event_free(e_sigint);
    event_free(e_sigusr1);
    event_free(e_update);
    event_base_free(base);

    /*
     * notify other base loop exit.
     */
    list_client_loopexit();

    /*
     * wait for all thread exit.
     */
    while(gl_client_info.head)
    {
        cinfo = gl_client_info.head->private_data;
        pthread_join(cinfo->pt, NULL);
    }

    log_msg(E_INFO, "Server is shutting down...");
    libevent_global_shutdown();

    return 0;
}

