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

int main(void) {
    context = hellow_init(PORT);

    hellow_set_default_root(context, "./website");

    signal(SIGINT, signal_handler);

    hellow_start_server(context);

    printf("Server running on http://localhost:%d\n", PORT);

    printf("Press the ENTER key to stop server: ");
    (void)getchar();

    hellow_stop(context);

    printf("Server stopped\n");

    return 0;
}