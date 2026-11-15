#include "mime.h"
#include <string.h>

const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html; charset=UTF-8";
    if (strcmp(ext, ".css") == 0)  return "text/css; charset=UTF-8";
    if (strcmp(ext, ".js") == 0)   return "application/javascript; charset=UTF-8";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".txt") == 0)  return "text/plain; charset=UTF-8";

    return "application/octet-stream";
}