#include "helloweb.h"

#include <string.h>
#include <stdio.h>

#define PORT 8080

static void home_callback(int socket, char *method, char *url, void *user_data)
{
    (void)user_data;

    hellow_send_response(socket, 200, "text/html",
                         "<html><body>"
                         "<h1>Milou Club</h1>"
                         "<p>Milou, known as Snowy in English, is Tintin's brave and loyal fox terrier. He's clever, curious, and always ready for adventure-especially if there's a bone involved.</p>"
                         "</body></html>");
}

static void about_callback(int socket, char *method, char *url, void *user_data)
{
    (void)user_data;

    hellow_send_response(socket, 200, "text/html",
                         "<html><body>"
                         "<h1>Milou Clud</h1>"
                         "<h2>About Page</h2>"
                         "<p>This website is hosted by Isak Jacobsson</p>"
                         "</body></html>");
}

int main(void)
{
    hellow_ctx *context = hellow_init(PORT);

    hellow_add_route(context, "/", home_callback, NULL);
    hellow_add_route(context, "/about", about_callback, NULL);

    hellow_start_server(context);

    printf("Server running on http://localhost:%d\n", PORT);

    printf("Press the ENTER key to stop server: ");
    (void)getchar();

    hellow_stop(context);

    printf("Server stopped\n");

    return 0;
}