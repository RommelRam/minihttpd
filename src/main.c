#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"

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

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    return EXIT_FAILURE;
    
    char www_root[2048];
    snprintf(www_root, sizeof(www_root), "%s/www", cwd);    
    printf("Servidor MiniHTTPd escuchando en el puerto %s...\n", port);

    run_server_loop(server_fd, www_root);

    close(server_fd);
    return EXIT_SUCCESS;
}