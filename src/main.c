#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PUERTO_PRUEBA "8080"
#define COLA_ESPERA   10

int main(void) {
    int msock, ssock;
    struct sockaddr_storage client_addr;
    socklen_t addrlen;
    char buffer[1024];

    msock = passiveTCP(PUERTO_PRUEBA, COLA_ESPERA);
    printf("Servidor de prueba del Paso 1 escuchando en el puerto %s...\n", PUERTO_PRUEBA);

    addrlen = sizeof(client_addr);
    ssock = accept(msock, (struct sockaddr *)&client_addr, &addrlen);
    if (ssock < 0) {
        errexit("Fallo en accept\n");
    }

    ssize_t n = read(ssock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("--- Solicitud recibida del cliente ---\n%s\n---------------------------------------\n", buffer);
    }

    const char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 18\r\nContent-Type: text/plain\r\n\r\nPaso 1 completado\n";
    write(ssock, resp, 83);

    close(ssock);
    close(msock);
    return 0;
}