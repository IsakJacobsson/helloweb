#include "helloweb.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 8080

hellow_ctx* context = NULL;

typedef struct {
    int request_count;
} example_data;

// Signal handler for SIGINT (Ctrl+C)
static void signal_handler(int signal) {
    if (context != NULL) {
        printf("\nCaught signal %d, stopping server...\n", signal);
        hellow_stop(context);
    }
    exit(0);  // Exit after cleaning up
}

void api_example_callback(hellow_response_context* response_ctx, void* user_data) {
    // Increment request count
    example_data* data = (example_data*)user_data;
    data->request_count++;

    // Array of dynamic greetings
    const char* greetings[] = {
        "Hello, World!",
        "Hi there!",
        "Greetings, traveler!",
        "Hey! Nice to see you!",
        "Salutations!"
    };
    int num_greetings = sizeof(greetings) / sizeof(greetings[0]);

    // Pick a random greeting
    srand((unsigned int)time(NULL) ^ rand());
    const char* message = greetings[rand() % num_greetings];

    // Build JSON response
    char* json_body = malloc(128);
    if (!json_body) return;
    snprintf(json_body, 128, "{\"message\":\"%s\",\"requestCount\":\"%d\"}", message, data->request_count);

    response_ctx->response->status_code  = 200;
    response_ctx->response->content_type = strdup("application/json");
    response_ctx->response->body         = json_body;
    response_ctx->response->body_length  = strlen(json_body);
}

int main(void) {
    context = hellow_init(PORT);

    hellow_set_default_root(context, "./react/dist");

    example_data* data = malloc(sizeof(example_data));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for example_data\n");
        return EXIT_FAILURE;
    }
    data->request_count = 0;
    hellow_add_route(context, "/api/hello", api_example_callback, data);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    hellow_start_server(context);

    printf("Server running on http://localhost:%d\n", PORT);

    printf("Press the ENTER key to stop server: ");
    (void)getchar();

    hellow_stop(context);

    printf("Server stopped\n");

    return 0;
}
