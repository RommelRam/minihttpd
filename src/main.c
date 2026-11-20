#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "server.h"

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <puerto>[-d]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *port = argv[1];

    int run_as_daemon = (argc == 3 && strcmp(argv[2], "-d") == 0);

    int server_fd = passivesock(port, "tcp", 128);
    if (server_fd < 0) {
        fprintf(stderr, "Error al iniciar el socket en el puerto %s\n", port);
        return EXIT_FAILURE;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        close(server_fd);
        return EXIT_FAILURE;
    }

    char www_root[2048];
    snprintf(www_root, sizeof(www_root), "%s/www", cwd);

    if(run_as_daemon) {
        printf("Servidor MiniHTTPd demonizado en el puerto %s...\n", port);
        fflush(stdout);
        daemonize_process();
    } else{
        printf("Servidor MiniHTTPd escuchando en el puerto %s...\n", port);
    }

    run_server_loop(server_fd, www_root);

    close(server_fd);
    return EXIT_SUCCESS;
}