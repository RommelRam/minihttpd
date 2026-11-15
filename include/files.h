#ifndef FILES_H
#define FILES_H

#include <sys/types.h>

ssize_t send_all(int fd, const void *buf, size_t len);
void send_error(int client_fd, int status_code, const char *status_text, const char *message);
void serve_file(int client_fd, const char *resolved_path);

#endif