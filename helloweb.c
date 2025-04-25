#include "helloweb.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_EVENTS  10

typedef struct {
    char* path;
    hellow_callback_function callback;
    void* user_data;
} hellow_route;

struct hellow_ctx {
    uint16_t port;
    int server_fd;
    struct sockaddr_in address;
    pthread_t accept_thread_id;
    int accept_stop_flag;
    hellow_route* routes;
    size_t route_count;
};

void hellow_stop(hellow_ctx* context) {
    if (!context)
        return;

    context->accept_stop_flag = 1;

    if (context->server_fd >= 0) {
        close(context->server_fd);
        context->server_fd = -1;
    }

    pthread_join(context->accept_thread_id, NULL);

    // Free paths for routes
    for (size_t i = 0; i < context->route_count; i++) free(context->routes[i].path);

    free(context->routes);

    free(context);
}

hellow_ctx* hellow_init(uint16_t port) {
    hellow_ctx* context = (hellow_ctx*)calloc(1, sizeof(hellow_ctx));
    context->port       = port;

    // Create socket
    context->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (context->server_fd == 0) {
        hellow_stop(context);
        return NULL;
    }

    // Set to socket to non blocking
    int flags = fcntl(context->server_fd, F_GETFL, 0);
    fcntl(context->server_fd, F_SETFL, flags | O_NONBLOCK);

    int opt = 1;
    if (setsockopt(context->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        hellow_stop(context);
        return NULL;
    }

    // Bind to port
    context->address.sin_family      = AF_INET;
    context->address.sin_addr.s_addr = INADDR_ANY;
    context->address.sin_port        = htons(port);
    if (bind(context->server_fd, (struct sockaddr*)&context->address, sizeof(context->address)) <
        0) {
        hellow_stop(context);
        return NULL;
    }

    return context;
}

int hellow_add_route(hellow_ctx* context,
                     char* path,
                     hellow_callback_function callback,
                     void* user_data) {
    if (!context || !path || !callback)
        return 0;

    hellow_route* new_routes =
        realloc(context->routes, sizeof(hellow_route) * (context->route_count + 1));
    if (!new_routes)
        return 0;

    context->routes                            = new_routes;
    context->routes[context->route_count].path = strdup(path);
    if (!context->routes[context->route_count].path)
        return 0;

    context->routes[context->route_count].callback  = callback;
    context->routes[context->route_count].user_data = user_data;
    context->route_count++;

    return 1;
}

static int hellow_send_response(hellow_response* response, int client_fd) {
    char header[BUFFER_SIZE];
    char* status_text;

    switch (response->status_code) {
        case 200:
            status_text = "OK";
            break;
        case 404:
            status_text = "Not Found";
            break;
        case 500:
            status_text = "Internal Server Error";
            break;
        default:
            status_text = "OK";
            break;
    }

    snprintf(header,
             BUFFER_SIZE,
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             response->status_code,
             status_text,
             response->content_type,
             response->body_length,
             response->body);

    write(client_fd, header, strlen(header));

    return 1;
}

static void handle_request(hellow_ctx* context, int client_fd) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    if (read(client_fd, buffer, BUFFER_SIZE - 1) <= 0) {
        return;
    }

    // Extract the method and path from the request
    char method[8] = {0}, path[1024] = {0};
    sscanf(buffer, "%7s %1023s", method, path);

    hellow_request request = {.method       = strdup(method),
                              .path         = strdup(path),
                              .query_string = NULL,
                              .headers      = NULL,
                              .body         = NULL};

    hellow_response response = {.status_code  = 0,
                                .content_type = NULL,
                                .body         = NULL,
                                .body_length  = 0};

    hellow_response_context response_context = {.client_fd       = client_fd,
                                                .request         = &request,
                                                .response        = &response,
                                                .manual_response = 0};

    // Call the correct callback
    int found_route = 0;
    for (int i = 0; i < context->route_count; i++) {
        if (strcmp(path, context->routes[i].path) == 0) {
            found_route = 1;
            context->routes[i].callback(&response_context, context->routes[i].user_data);
            break;
        }
    }

    if (response_context.manual_response)
        goto cleanup;

    if (!found_route) {
        char body_404[] = "<html><body><h1>404 Not Found</h1></body></html>";

        response_context.response->status_code  = 404;
        response_context.response->content_type = strdup("text/html");
        response_context.response->body         = strdup(body_404);
        response_context.response->body_length  = strlen(body_404);
    }
    // Check if response is incomplete, return a 500 if so
    else if (response_context.response->content_type == NULL ||
             response_context.response->body == NULL ||
             response_context.response->status_code == 0) {
        char body_500[] = "<html><body><h1>500 Internal Server Error</h1></body></html>";

        // Free memory allocated by the callback
        free(response_context.response->content_type);
        free(response_context.response->body);

        response_context.response->status_code  = 500;
        response_context.response->content_type = strdup("text/html");
        response_context.response->body         = strdup(body_500);
        response_context.response->body_length  = strlen(body_500);
    }

    hellow_send_response(response_context.response, client_fd);

cleanup:
    free(request.method);
    free(request.path);
    free(response_context.response->content_type);
    free(response_context.response->body);
}

static void* accept_thread(void* arg) {
    hellow_ctx* context = (hellow_ctx*)arg;

    int epoll_fd          = epoll_create1(0);
    struct epoll_event ev = {.events = EPOLLIN, .data.fd = context->server_fd};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, context->server_fd, &ev);

    struct epoll_event events[MAX_EVENTS];

    int addrlen = sizeof(context->address);
    while (!context->accept_stop_flag) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == context->server_fd) {
                // Accept the connection
                int client_fd                = accept(context->server_fd, NULL, NULL);
                struct epoll_event client_ev = {.events = EPOLLIN | EPOLLET, .data.fd = client_fd};
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);

            } else {
                // Read from client
                handle_request(context, events[i].data.fd);
                close(events[i].data.fd);
            }
        }
    }

    close(epoll_fd);
    return NULL;
}

int hellow_start_server(hellow_ctx* context) {
    if (listen(context->server_fd, SOMAXCONN) < 0) {
        hellow_stop(context);
        return 0;
    }

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, accept_thread, (void*)context) != 0) {
        hellow_stop(context);
        return 0;
    }

    context->accept_thread_id = thread_id;
    return 1;
}