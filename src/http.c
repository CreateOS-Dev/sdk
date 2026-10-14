#include "createos/http.h"

static int is_token(unsigned char ch)
{
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9'))
        return 1;
    switch (ch) {
    case '!': case '#': case '$': case '%': case '&': case '\'':
    case '*': case '+': case '-': case '.': case '^': case '_':
    case '`': case '|': case '~':
        return 1;
    default:
        return 0;
    }
}

static unsigned char ascii_lower(unsigned char ch)
{
    return ch >= 'A' && ch <= 'Z' ? (unsigned char)(ch + ('a' - 'A')) : ch;
}

static int span_equal_ci(const unsigned char* data, size_t length,
                         const char* expected)
{
    size_t i = 0;
    while (expected[i]) {
        if (i >= length ||
            ascii_lower(data[i]) != ascii_lower((unsigned char)expected[i]))
            return 0;
        i++;
    }
    return i == length;
}

int createos_http_header_name_is(const unsigned char* data, size_t length,
                                 const char* name)
{
    if (!data || !name)
        return 0;
    return span_equal_ci(data, length, name);
}

static size_t find_crlf(const unsigned char* data, size_t start, size_t end)
{
    for (size_t i = start; i + 1 < end; i++) {
        if (data[i] == '\r' && data[i + 1] == '\n')
            return i;
        if (data[i] == '\n' || data[i] == '\r')
            return (size_t)-1;
    }
    if (end > start && data[end - 1] == '\r')
        return (size_t)-2;
    return (size_t)-2;
}

int createos_http_parse_request(const unsigned char* data, size_t length,
                                createos_http_request_t* request)
{
    size_t request_line_end;
    size_t method_end;
    size_t target_end;
    size_t version_end;
    size_t cursor;
    size_t header_end = 0;
    unsigned int header_count = 0;
    size_t content_length = 0;
    int has_content_length = 0;
    int has_host = 0;

    if (!data || !request)
        return -1;
    if (length > CREATEOS_HTTP_HEADER_BYTES_MAX +
                 CREATEOS_HTTP_REQUEST_LINE_MAX)
        return -1;
    request->method_offset = 0;
    request->method_length = 0;
    request->target_offset = 0;
    request->target_length = 0;
    request->body_offset = 0;
    request->body_length = 0;
    request->content_length = 0;
    request->header_count = 0;
    request->http_minor_version = 0;
    request->has_content_length = 0;
    request->has_host = 0;

    request_line_end = find_crlf(data, 0, length);
    if (request_line_end == (size_t)-1)
        return -1;
    if (request_line_end == (size_t)-2)
        return length > CREATEOS_HTTP_REQUEST_LINE_MAX ? -1 : 0;
    if (request_line_end > CREATEOS_HTTP_REQUEST_LINE_MAX)
        return -1;

    method_end = 0;
    while (method_end < request_line_end && data[method_end] != ' ') {
        if (!is_token(data[method_end]))
            return -1;
        method_end++;
    }
    if (!method_end || method_end == request_line_end)
        return -1;

    request->target_offset = method_end + 1;
    target_end = request->target_offset;
    while (target_end < request_line_end && data[target_end] != ' ') {
        unsigned char ch = data[target_end];
        if (ch < 0x21 || ch > 0x7e || ch == '#' || ch == '\\')
            return -1;
        target_end++;
    }
    if (target_end == request->target_offset ||
        target_end == request_line_end || data[request->target_offset] != '/')
        return -1;
    for (size_t i = target_end + 1; i < request_line_end; i++)
        if (data[i] == ' ')
            return -1;

    version_end = target_end + 1;
    if (request_line_end - version_end != 8 ||
        data[version_end] != 'H' || data[version_end + 1] != 'T' ||
        data[version_end + 2] != 'T' || data[version_end + 3] != 'P' ||
        data[version_end + 4] != '/' || data[version_end + 5] != '1' ||
        data[version_end + 6] != '.' ||
        (data[version_end + 7] != '0' && data[version_end + 7] != '1'))
        return -1;
    request->http_minor_version = data[version_end + 7] - '0';
    request->method_length = method_end;
    request->target_length = target_end - request->target_offset;

    cursor = request_line_end + 2;
    while (cursor + 1 < length) {
        size_t line_end;
        size_t colon;
        size_t value_start;
        size_t value_end;
        line_end = find_crlf(data, cursor, length);
        if (line_end == (size_t)-1)
            return -1;
        if (line_end == (size_t)-2) {
            if (length - cursor > CREATEOS_HTTP_HEADER_BYTES_MAX)
                return -1;
            return 0;
        }
        if (line_end == cursor) {
            header_end = line_end + 2;
            break;
        }
        if (line_end - (request_line_end + 2) >
                CREATEOS_HTTP_HEADER_BYTES_MAX ||
            ++header_count > CREATEOS_HTTP_HEADER_COUNT_MAX ||
            data[cursor] == ' ' || data[cursor] == '\t')
            return -1;

        colon = cursor;
        while (colon < line_end && data[colon] != ':') {
            if (!is_token(data[colon]))
                return -1;
            colon++;
        }
        if (colon == cursor || colon == line_end)
            return -1;
        value_start = colon + 1;
        while (value_start < line_end &&
               (data[value_start] == ' ' || data[value_start] == '\t'))
            value_start++;
        value_end = line_end;
        while (value_end > value_start &&
               (data[value_end - 1] == ' ' ||
                data[value_end - 1] == '\t'))
            value_end--;
        for (size_t i = value_start; i < value_end; i++)
            if ((data[i] < 0x20 && data[i] != '\t') || data[i] > 0x7e)
                return -1;

        if (createos_http_header_name_is(data + cursor, colon - cursor,
                                         "host")) {
            if (has_host || value_start == value_end)
                return -1;
            has_host = 1;
        } else if (createos_http_header_name_is(
                       data + cursor, colon - cursor, "content-length")) {
            size_t parsed = 0;
            if (has_content_length || value_start == value_end)
                return -1;
            for (size_t i = value_start; i < value_end; i++) {
                size_t digit;
                if (data[i] < '0' || data[i] > '9')
                    return -1;
                digit = data[i] - '0';
                if (parsed > ((size_t)-1 - digit) / 10)
                    return -1;
                parsed = parsed * 10 + digit;
            }
            has_content_length = 1;
            content_length = parsed;
        } else if (createos_http_header_name_is(
                       data + cursor, colon - cursor, "transfer-encoding")) {
            return -1;
        }
        cursor = line_end + 2;
    }
    if (!header_end) {
        if (length - (request_line_end + 2) > CREATEOS_HTTP_HEADER_BYTES_MAX)
            return -1;
        return 0;
    }
    if (header_end > request_line_end + 2 + CREATEOS_HTTP_HEADER_BYTES_MAX)
        return -1;
    if (request->http_minor_version == 1 && !has_host)
        return -1;
    request->header_count = header_count;
    request->body_offset = header_end;
    request->has_content_length = has_content_length;
    request->has_host = has_host;
    request->content_length = content_length;
    if (!has_content_length)
        return header_end == length ? 1 : -1;
    if (length - header_end < content_length)
        return 0;
    request->body_length = content_length;
    return 1;
}
