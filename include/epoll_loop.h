#ifndef EPOLL_LOOP_H
#define EPOLL_LOOP_H

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096

int set_nonblocking(int fd);
void run_epoll_loop(int server_fd, const char *www_root);

#endif