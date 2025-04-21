#include "helloweb.h"

#define BUFFER_SIZE 4096
#define MAX_EVENTS 10

void hellow_stop(hellow_ctx *context)
{
    if (!context)
        return;

    context->accept_stop_flag = 1;

    if (context->server_fd >= 0)
    {
        close(context->server_fd);
        context->server_fd = -1;
    }

    pthread_join(context->accept_thread_id, NULL);

    // Free urls for routes
    for (size_t i = 0; i < context->route_count; i++)
        free(context->routes[i].url);

    free(context->routes);

    free(context);
}

hellow_ctx *hellow_init(uint16_t port)
{
    hellow_ctx *context = (hellow_ctx *)calloc(1, sizeof(hellow_ctx));
    context->port = port;

    // Create socket
    context->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (context->server_fd == 0)
    {
        hellow_stop(context);
        return NULL;
    }

    int opt = 1;
    if (setsockopt(context->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        hellow_stop(context);
        return NULL;
    }

    // Bind to port
    context->address.sin_family = AF_INET;
    context->address.sin_addr.s_addr = INADDR_ANY;
    context->address.sin_port = htons(port);
    if (bind(context->server_fd, (struct sockaddr *)&context->address, sizeof(context->address)) < 0)
    {
        hellow_stop(context);
        return NULL;
    }

    return context;
}

int hellow_add_route(hellow_ctx *context, char *url, hellow_callback_function callback, void *user_data)
{
    if (!context || !url || !callback)
        return 0;

    hellow_route *new_routes = realloc(context->routes, sizeof(hellow_route) * (context->route_count + 1));
    if (!new_routes)
        return 0;

    context->routes = new_routes;
    context->routes[context->route_count].url = strdup(url);
    if (!context->routes[context->route_count].url)
        return 0;

    context->routes[context->route_count].callback = callback;
    context->routes[context->route_count].user_data = user_data;
    context->route_count++;

    return 1;
}

int hellow_send_response(int socket, unsigned int status_code, char *content_type, char *body)
{
    char header[BUFFER_SIZE];
    const char *status_text;

    switch (status_code)
    {
    case 200:
        status_text = "OK";
        break;
    case 404:
        status_text = "Not Found";
        break;
    default:
        status_text = "OK";
        break;
    }

    int body_len = strlen(body);
    snprintf(header, BUFFER_SIZE,
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             status_code, status_text, content_type, body_len, body);

    write(socket, header, strlen(header));

    return 1;
}

static void handle_request(hellow_ctx *context, int client_socket)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    read(client_socket, buffer, BUFFER_SIZE - 1);

    // Extract the path from the request
    char method[8], url[1024];
    sscanf(buffer, "%s %1023s", method, url);

    // Call the correct callback
    int found_route = 0;
    for (int i = 0; i < context->route_count; i++)
    {
        if (strcmp(url, context->routes[i].url) == 0)
        {
            context->routes[i].callback(client_socket,
                                        method, context->routes[i].url,
                                        context->routes[i].user_data);
        }
    }
    if (!found_route)
    {
        hellow_send_response(client_socket, 404, "text/html", "<html><body><h1>404 Not Found</h1></body></html>");
    }
}

static void *accept_thread(void *arg)
{
    hellow_ctx *context = (hellow_ctx *)arg;

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.fd = context->server_fd};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, context->server_fd, &ev);

    struct epoll_event events[MAX_EVENTS];

    int addrlen = sizeof(context->address);
    while (!context->accept_stop_flag)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < n; ++i)
        {
            if (events[i].data.fd == context->server_fd)
            {
                // Accept new connection
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(context->server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd < 0)
                {
                    if (errno == EINTR || errno == EAGAIN)
                        continue; // harmless, retry
                    if (context->accept_stop_flag)
                        break;
                    continue;
                }
                struct epoll_event client_ev = {
                    .events = EPOLLIN | EPOLLET,
                    .data.fd = client_fd};
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
            }
            else
            {
                // Read from client
                handle_request(context, events[i].data.fd);
                close(events[i].data.fd);
            }
        }
    }

    close(epoll_fd);
    return NULL;
}

int hellow_start_server(hellow_ctx *context)
{
    if (listen(context->server_fd, 3) < 0)
    {
        hellow_stop(context);
        return 0;
    }

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, accept_thread, (void *)context) != 0)
    {
        hellow_stop(context);
        return 0;
    }

    context->accept_thread_id = thread_id;
    return 1;
}