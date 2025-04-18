#include <string.h>
#include "helloweb.h"

#define PORT 8080

static int my_callback(int socket, char *method, char *url, void *user_data)
{
    (void)user_data;

    if (strcmp(method, "GET") != 0)
    {
        return 0;
    }

    if (strcmp(url, "/") == 0)
    {
        hellow_send_response(socket, 200, "text/html",
                             "<html><body>"
                             "<h1>Welcome to the Milou Fan Club</h1>"
                             "<p>Milou, known as Snowy in English, is Tintin's brave and loyal fox terrier. He's clever, curious, and always ready for adventure-especially if there's a bone involved.</p>"
                             "</body></html>");
    }
    else if (strcmp(url, "/about") == 0 || strcmp(url, "/about/") == 0)
    {
        hellow_send_response(socket, 200, "text/html", "<html><body><h1>About Page</h1></body></html>");
    }
    else
    {
        hellow_send_response(socket, 404, "text/html", "<html><body><h1>404 Not Found</h1></body></html>");
    }

    return 1;
}

int main(void)
{
    hellow_ctx *context = hellow_init(PORT, my_callback, NULL);

    hellow_start_server(context);

    printf("Press the ENTER key to stop server: ");
    (void)getchar();

    hellow_stop(context);

    return 0;
}