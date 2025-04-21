#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#pragma once

typedef void (*hellow_callback_function)(int socket, char *method, char *url, void *user_data);

typedef struct
{
    char *url;
    hellow_callback_function callback;
    void *user_data;
} hellow_route;

typedef struct hellow_ctx
{
    uint16_t port;
    int server_fd;
    struct sockaddr_in address;
    pthread_t accept_thread_id;
    int accept_stop_flag;
    hellow_route *routes;
    size_t route_count;
} hellow_ctx;

void hellow_stop(hellow_ctx *context);

hellow_ctx *hellow_init(uint16_t port);

int hellow_add_route(hellow_ctx *context, char *url, hellow_callback_function callback, void *user_data);

int hellow_send_response(int socket, unsigned int status_code, char *content_type, char *body);

int hellow_start_server(hellow_ctx *context);