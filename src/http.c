#define _POSIX_C_SOURCE 200809L
#include "http.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>


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

    serve_file(client_fd, resolved_path);
}