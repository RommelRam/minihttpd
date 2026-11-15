#include "files.h"
#include "mime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t total = 0;
    const char *ptr = (const char *)buf;
    while (total < len) {
        ssize_t sent = write(fd, ptr + total, len - total);
        if (sent <= 0) {
            if (sent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) continue;
            return -1;
        }
        total += (size_t)sent;
    }
    return (ssize_t)total;
}

void send_error(int client_fd, int status_code, const char *status_text, const char *message) {
    char header[512];
    size_t body_len = strlen(message);
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
        status_code, status_text, body_len);
    
    if (header_len > 0) {
        send_all(client_fd, header, (size_t)header_len);
        send_all(client_fd, message, body_len);
    }
}

void serve_file(int client_fd, const char *resolved_path) {
    struct stat st;
    if (stat(resolved_path, &st) == -1 || !S_ISREG(st.st_mode)) {
        send_error(client_fd, 404, "Not Found", "404 Not Found\n");
        return;
    }

    FILE *fp = fopen(resolved_path, "rb");
    if (!fp) {
        send_error(client_fd, 500, "Internal Server Error", "500 Internal Server Error\n");
        return;
    }

    const char *mime = get_mime_type(resolved_path);
    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %lld\r\nConnection: close\r\n\r\n",
        mime, (long long)st.st_size);

    if (header_len <= 0 || send_all(client_fd, header, (size_t)header_len) == -1) {
        fclose(fp);
        return;
    }

    char file_buf[8192];
    size_t bytes_read;
    while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        if (send_all(client_fd, file_buf, bytes_read) == -1) break;
    }
    fclose(fp);
}