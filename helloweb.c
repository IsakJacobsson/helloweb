#include "helloweb.h"

#define BUFFER_SIZE 4096

void hellow_stop(hellow_ctx *context)
{
    if (context->server_fd >= 0)
    {
        close(context->server_fd);
    }

    // Stop accept thread
    context->accept_stop_flag = 1;
    pthread_join(context->accept_thread_id, NULL);

    // Free urls for routes
    for (size_t i = 0; i < context->route_count; i++)
    {
        free(context->routes[i].url);
    }

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

    close(client_socket);
}

static void *accept_thread(void *arg)
{
    hellow_ctx *context = (hellow_ctx *)arg;

    int addrlen = sizeof(context->address);
    while (!context->accept_stop_flag)
    {
        int client_socket = accept(context->server_fd, (struct sockaddr *)&context->address, (socklen_t *)&addrlen);
        if (client_socket < 0)
        {
            if (context->accept_stop_flag)
                break;
            hellow_stop(context);
            return NULL;
        }

        handle_request(context, client_socket);
    }

    return NULL;
}

void hellow_start_server(hellow_ctx *context)
{
    if (listen(context->server_fd, 3) < 0)
    {
        hellow_stop(context);
    }

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, accept_thread, (void *)context) != 0)
    {
        hellow_stop(context);
        return;
    }

    context->accept_thread_id = thread_id;
}