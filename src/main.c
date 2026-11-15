#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "epoll_loop.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *port = argv[1];
    int qlen = 128;

    int server_fd = passivesock(port, "tcp", qlen);
    if (server_fd < 0) {
        fprintf(stderr, "Error al iniciar el socket en el puerto %s\n", port);
        return EXIT_FAILURE;
    }

    printf("Servidor MiniHTTPd escuchando en el puerto %s...\n", port);

    run_epoll_loop(server_fd, "./www");

    close(server_fd);
    return EXIT_SUCCESS;
}