#include "helloweb.h"

#include <stdio.h>
#include <string.h>

#define PORT 8080

static void home_callback(hellow_response_context* response_ctx, void* user_data) {
    (void)user_data;

    char body[] =
        "<html><body>"
        "<h1>Milou Club</h1>"
        "<p>Milou, known as Snowy in English, is Tintin's brave and loyal fox terrier. He's "
        "clever, curious, and always ready for adventure-especially if there's a bone involved.</p>"
        "</body></html>";

    response_ctx->response->status_code  = 200;
    response_ctx->response->content_type = strdup("text/html");
    response_ctx->response->body         = strdup(body);
    if (!response_ctx->response->body) {
        return;
    }
    response_ctx->response->body_length = strlen(body);
}

static void about_callback(hellow_response_context* response_ctx, void* user_data) {
    (void)user_data;

    char body[] =
        "<h1>Milou Clud</h1>"
        "<h2>About Page</h2>"
        "<p>This website is hosted by Isak Jacobsson</p>"
        "</body></html>";

    response_ctx->response->status_code  = 200;
    response_ctx->response->content_type = strdup("text/html");
    response_ctx->response->body         = strdup(body);
    if (!response_ctx->response->body) {
        return;
    }
    response_ctx->response->body_length = strlen(body);
}

int main(void) {
    hellow_ctx* context = hellow_init(PORT);

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