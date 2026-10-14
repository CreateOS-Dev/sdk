#ifndef CREATEOS_HTTP_H
#define CREATEOS_HTTP_H

#include <stddef.h>

#define CREATEOS_HTTP_REQUEST_LINE_MAX 2048
#define CREATEOS_HTTP_HEADER_BYTES_MAX 8192
#define CREATEOS_HTTP_HEADER_COUNT_MAX 64

typedef struct {
    size_t method_offset;
    size_t method_length;
    size_t target_offset;
    size_t target_length;
    size_t body_offset;
    size_t body_length;
    size_t content_length;
    unsigned int header_count;
    unsigned int http_minor_version;
    int has_content_length;
    int has_host;
} createos_http_request_t;

/* Returns 1 for a complete request, 0 for incomplete input, and -1 if invalid. */
int createos_http_parse_request(const unsigned char* data, size_t length,
                                createos_http_request_t* request);

int createos_http_header_name_is(const unsigned char* data, size_t length,
                                 const char* name);

#endif
