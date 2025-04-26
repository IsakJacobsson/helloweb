#include "helloweb.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

hellow_ctx* context = NULL;

typedef struct {
    char* filename;
    char* content_type;
} callback_data;

// Signal handler for SIGINT (Ctrl+C)
static void signal_handler(int signal) {
    if (context != NULL) {
        printf("\nCaught signal %d, stopping server...\n", signal);
        hellow_stop(context);
    }
    exit(0);  // Exit after cleaning up
}

static void file_callback(hellow_response_context* response_ctx, void* user_data) {
    callback_data* file_info = (callback_data*)user_data;

    FILE* f = fopen(file_info->filename, "rb");
    if (!f)
        return;

    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char* file_data = malloc(file_size);
    if (!file_data) {
        fclose(f);
        return;
    }

    fread(file_data, 1, file_size, f);
    fclose(f);

    response_ctx->response->status_code  = 200;
    response_ctx->response->content_type = strdup(file_info->content_type);

    // Important: Copy the image into response->body
    response_ctx->response->body = malloc(file_size);
    memcpy(response_ctx->response->body, file_data, file_size);
    response_ctx->response->body_length = file_size;

    free(file_data);
}

int main(void) {
    context = hellow_init(PORT);

    callback_data index_html_info = {.filename     = "./website/index.html",
                                     .content_type = "text/html"};
    callback_data milou1_info     = {.filename     = "./website/images/milou1.jpeg",
                                     .content_type = "image/jpeg"};
    callback_data milou2_info     = {.filename     = "./website/images/milou2.png",
                                     .content_type = "image/png"};
    callback_data milou3_info     = {.filename     = "./website/images/milou3.png",
                                     .content_type = "image/png"};
    callback_data milou4_info     = {.filename     = "./website/images/milou4.jpg",
                                     .content_type = "image/jpg"};
    callback_data milou5_info     = {.filename     = "./website/images/milou5.jpg",
                                     .content_type = "image/jpg"};

    hellow_add_route(context, "/", file_callback, &index_html_info);
    hellow_add_route(context, "/milou1", file_callback, &milou1_info);
    hellow_add_route(context, "/milou2", file_callback, &milou2_info);
    hellow_add_route(context, "/milou3", file_callback, &milou3_info);
    hellow_add_route(context, "/milou4", file_callback, &milou4_info);
    hellow_add_route(context, "/milou5", file_callback, &milou5_info);

    signal(SIGINT, signal_handler);

    hellow_start_server(context);

    printf("Server running on http://localhost:%d\n", PORT);

    printf("Press the ENTER key to stop server: ");
    (void)getchar();

    hellow_stop(context);

    printf("Server stopped\n");

    return 0;
}