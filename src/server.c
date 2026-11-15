#define _POSIX_C_SOURCE 200809L
#include "server.h"
#include "http.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
 
#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096


int passivesock(const char *service, const char *transport, int qlen) {
    struct addrinfo hints, *res, *rp;
  
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = (strcmp(transport, "udp") == 0) ? SOCK_DGRAM : SOCK_STREAM;   
    hints.ai_flags = AI_PASSIVE;     

    if (getaddrinfo(NULL, service, &hints, &res) != 0) return -1;

    int s = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s == -1) continue;

        int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(s, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(s);
    }
    freeaddrinfo(res);
    
    if (rp == NULL || (hints.ai_socktype == SOCK_STREAM && listen(s, qlen) == -1)) return -1;
    return s;
}

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void run_server_loop(int server_fd, const char *www_root) {
    if (set_nonblocking(server_fd) == -1) exit(EXIT_FAILURE);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) exit(EXIT_FAILURE);

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) exit(EXIT_FAILURE);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                while (1) {
                    struct sockaddr_storage client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
                    if (client_fd == -1) break;
                    if (set_nonblocking(client_fd) == -1) { close(client_fd); continue; }
                    
                    struct epoll_event client_ev;
                    client_ev.events = EPOLLIN | EPOLLET;
                    client_ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
                }
            } else if (events[i].events & EPOLLIN) {
                int client_fd = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    handle_http_request(client_fd, buffer, www_root);
                }
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                close(client_fd);
            }
        }
    }
    close(epoll_fd);
}