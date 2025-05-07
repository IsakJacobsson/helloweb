#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct hellow_ctx hellow_ctx;

typedef struct {
    char* method;
    char* path;
    char* query_string;
    char* headers;
    char* body;
} hellow_request;

typedef struct {
    int status_code;
    char* content_type;
    char* body;
    size_t body_length;
} hellow_response;

typedef struct {
    int client_fd;
    hellow_request* request;
    hellow_response* response;
    int manual_response;
} hellow_response_context;

typedef void (*hellow_callback_function)(hellow_response_context* response_ctx, void* user_data);

void hellow_stop(hellow_ctx* context);

hellow_ctx* hellow_init(uint16_t port);

int hellow_add_route(hellow_ctx* context,
                     char* path,
                     hellow_callback_function callback,
                     void* user_data);

int hellow_start_server(hellow_ctx* context);

void hellow_set_default_root(hellow_ctx* context, const char* default_root);