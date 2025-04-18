#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#pragma once

typedef int (*hellow_callback_function)(int socket, char *method, char *url, void *user_data);

typedef struct hellow_ctx
{
    uint16_t port;
    int server_fd;
    struct sockaddr_in address;
    hellow_callback_function callback_function;
    void *callback_user_data;
    pthread_t accept_thread_id;
    int accept_stop_flag;
} hellow_ctx;

void hellow_stop(hellow_ctx *context);

hellow_ctx *hellow_init(uint16_t port, hellow_callback_function callback_function, void *callback_user_data);

int hellow_send_response(int socket, unsigned int status_code, char *content_type, char *body);

void hellow_start_server(hellow_ctx *context);