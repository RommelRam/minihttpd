#define _POSIX_C_SOURCE 200809L
#include "server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}


int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int passivesock(const char *service, const char *transport, int qlen) {
    struct addrinfo hints, *res, *rp;
    int s = -1, status;
    int optval = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    
    hints.ai_flags = AI_PASSIVE;     

    if (strcmp(transport, "udp") == 0) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    } else {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }


    status = getaddrinfo(NULL, service, &hints, &res);
    if (status != 0)
        errexit("passivesock: error en getaddrinfo: %s\n", gai_strerror(status));

    /* Iterar la lista enlazada de direcciones devueltas */
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s < 0) continue;

        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(s, rp->ai_addr, rp->ai_addrlen) == 0)
            break; 

        close(s);
        s = -1;
    }

    freeaddrinfo(res); 

    if (s < 0)
        errexit("passivesock: no se pudo enlazar el socket al puerto %s\n", service);

    if (hints.ai_socktype == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("passivesock: error en listen: %s\n", strerror(errno));

    return s;
}


int passiveTCP(const char *service, int qlen) {
    return passivesock(service, "tcp", qlen);
}