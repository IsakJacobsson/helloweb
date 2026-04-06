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

/**
 * User defined callback function for handling requests.
 *
 * @param response_ctx Context containing request and response information.
 * @param user_data User-defined data passed during route registration.
 */
typedef void (*hellow_callback_function)(hellow_response_context* response_ctx, void* user_data);

/**
 * Initializes the Helloweb server on the specified port.
 *
 * @param port The port number on which the server will listen for incoming connections.
 * @return A pointer to the initialized server context, or NULL on failure.
 */
hellow_ctx* hellow_init(uint16_t port);

/**
 * Sets the default root directory for serving static files.
 *
 * @param context The server context returned by hellow_init.
 * @param default_root The path to the default root directory for static files.
 */
void hellow_set_default_root(hellow_ctx* context, const char* default_root);

/**
 * Registers a route with the specified path and callback function. The callback will be invoked when a request matches the path.
 *
 * @param context The server context returned by hellow_init.
 * @param path The URL path to register (e.g., "/api/get_info").
 * @param callback The user-defined callback function to handle requests for the specified path.
 * @param user_data User-defined data that will be passed to the callback function when a request is received for the registered path.
 * @return 1 on success, or 0 on failure.
 */
int hellow_add_route(hellow_ctx* context,
                     char* path,
                     hellow_callback_function callback,
                     void* user_data);

/**
 * Starts the Helloweb server, allowing it to accept incoming connections and handle requests based on the registered routes. This function does not block the calling thread, allowing the server to run in the background while the main thread can perform other tasks or wait for a shutdown signal.
 *
 * @param context The server context returned by hellow_init.
 * @return 1 on success, or 0 on failure.
 */
int hellow_start(hellow_ctx* context);

/**
 * Stops the Helloweb server, closing the listening socket and cleaning up any resources allocated for the server. After calling this function, the server will no longer accept incoming connections or handle requests.
 *
 * @param context The server context returned by hellow_init.
 */
void hellow_stop(hellow_ctx* context);
