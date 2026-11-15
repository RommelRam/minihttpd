#define _POSIX_C_SOURCE 200809L
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

static const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html; charset=UTF-8";
    if (strcmp(ext, ".css") == 0)  return "text/css; charset=UTF-8";
    if (strcmp(ext, ".js") == 0)   return "application/javascript; charset=UTF-8";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)  return "image/gif";
    if (strcmp(ext, ".svg") == 0)  return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
    if (strcmp(ext, ".txt") == 0)  return "text/plain; charset=UTF-8";

    return "application/octet-stream";
}

static ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t total = 0;
    const char *ptr = (const char *)buf;

    while (total < len) {
        ssize_t sent = write(fd, ptr + total, len - total);
        if (sent <= 0) {
            if (sent < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
            return -1;
        }
        total += (size_t)sent;
    }
    return (ssize_t)total;
}

static void send_error(int client_fd, int status_code, const char *status_text, const char *message) {
    char header[512];
    size_t body_len = strlen(message);

    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, body_len);

    if (header_len > 0) {
        send_all(client_fd, header, (size_t)header_len);
        send_all(client_fd, message, body_len);
    }
}

void handle_http_request(int client_fd, const char *raw_request, const char *www_root) {
    char method[16];
    char uri[1024];
    char version[16];

    if (sscanf(raw_request, "%15s %1023s %15s", method, uri, version) != 3) {
        send_error(client_fd, 400, "Bad Request", "400 Bad Request\n");
        return;
    }

    if (strcmp(method, "GET") != 0) {
        send_error(client_fd, 405, "Method Not Allowed", "405 Method Not Allowed\n");
        return;
    }

    char *query = strchr(uri, '?');
    if (query) *query = '\0';

    char rel_path[1050];
    if (strcmp(uri, "/") == 0) {
        snprintf(rel_path, sizeof(rel_path), "%s/index.html", www_root);
    } else {
        snprintf(rel_path, sizeof(rel_path), "%s%s", www_root, uri);
    }

    char resolved_root[PATH_MAX];
    char resolved_path[PATH_MAX];

    if (!realpath(www_root, resolved_root) || !realpath(rel_path, resolved_path)) {
        send_error(client_fd, 404, "Not Found", "404 Not Found\n");
        return;
    }

    size_t root_len = strlen(resolved_root);
    if (strncmp(resolved_path, resolved_root, root_len) != 0 ||
        (resolved_path[root_len] != '/' && resolved_path[root_len] != '\0')) {
        send_error(client_fd, 403, "Forbidden", "403 Forbidden\n");
        return;
    }

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
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lld\r\n"
        "Connection: close\r\n"
        "\r\n",
        mime, (long long)st.st_size);

    if (header_len <= 0 || send_all(client_fd, header, (size_t)header_len) == -1) {
        fclose(fp);
        return;
    }

    char file_buf[8192];
    size_t bytes_read;
    while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        if (send_all(client_fd, file_buf, bytes_read) == -1) {
            break;
        }
    }

    fclose(fp);
}