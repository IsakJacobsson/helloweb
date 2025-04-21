#pragma once

#include <stdint.h>
#include <stddef.h>

typedef void (*hellow_callback_function)(int socket, char *method, char *url, void *user_data);

typedef struct hellow_ctx hellow_ctx;

void hellow_stop(hellow_ctx *context);

hellow_ctx *hellow_init(uint16_t port);

int hellow_add_route(hellow_ctx *context, char *url, hellow_callback_function callback, void *user_data);

int hellow_send_response(int socket, unsigned int status_code, char *content_type, char *body);

int hellow_start_server(hellow_ctx *context);