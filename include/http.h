#ifndef HTTP_H
#define HTTP_H

void handle_http_request(int client_fd, const char *raw_request, const char *www_root);

#endif // HTTP_H