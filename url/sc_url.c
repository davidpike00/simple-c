/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sc_url.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

struct sc_url *sc_url_create(const char *str)
{
    const char *s1 = "%.*s%.*s%.*s%.*s%.*s%.*s%.*s%.*s";
    const char *s2 = "%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c";
    const char *authority = "//";

    unsigned long val;
    size_t len, full_len, parts_len;
    size_t scheme_len = 0, authority_len = 0, userinfo_len = 0;
    size_t host_len = 0, port_len = 0, path_len = 0;
    size_t query_len = 0, fragment_len = 0;

    char *scheme = "", *userinfo = "", *host = "", *port = "";
    char *path = "", *query = "", *fragment = "";
    char *ptr, *dest, *parse_end;
    char* pos = (char*) str;
    struct sc_url *url;

    if (str == NULL || (ptr = strstr(pos, ":")) == NULL) {
        return NULL;
    }

    scheme = pos;
    scheme_len = ptr - str + 1;
    pos = ptr + 1;

    if (*pos == '/' && *(pos + 1) == '/') {
        authority_len = 2;
        pos += authority_len;

        ptr = strchr(pos, '@');
        if (ptr != NULL) {
            userinfo = pos;
            userinfo_len = ptr - pos + strlen("@");
            pos = ptr + 1;
        }

        ptr = pos + strcspn(pos, *pos == '[' ? "]" : ":/?#");
        host = pos;
        host_len = ptr - pos + (*host == '[');
        pos = host + host_len;

        if (*host == '[' && *(host + host_len - 1) != ']') {
            return NULL;
        }

        ptr = strchr(pos, ':');
        if (ptr != NULL) {
            if (*(ptr + 1) == '\0') {
                return NULL;
            }

            errno = 0;
            val = strtoul(ptr + 1, &parse_end, 10);
            if (errno != 0 || val > 65536) {
                return NULL;
            }

            port = ptr;
            port_len = parse_end - ptr;
            pos = port + port_len;
        }
    }

    path = pos;
    path_len = strcspn(path, "?#");
    pos = path + path_len;

    ptr = strchr(pos, '?');
    if (ptr != NULL) {
        query = ptr;
        query_len = strcspn(query, "#");
        pos = query + query_len;
    }

    if (*pos == '#') {
        fragment = pos;
        fragment_len = strlen(pos);
    }

    full_len = scheme_len + authority_len + userinfo_len + host_len + port_len +
               path_len + query_len + fragment_len + 1;

    parts_len = full_len - authority_len;
    parts_len += 7; // NULL characters for each part.
    parts_len -= (scheme_len != 0);
    parts_len -= (userinfo_len != 0);
    parts_len -= (port_len != 0);
    parts_len -= (query_len != 0);
    parts_len -= (fragment_len != 0);

    url = sc_url_malloc(sizeof(struct sc_url) + parts_len + full_len);
    if (url == NULL) {
        return NULL;
    }

    len = snprintf(url->buf, full_len, s1, scheme_len, scheme, authority_len,
                   authority, userinfo_len, userinfo, host_len, host, port_len,
                   port, path_len, path, query_len, query, fragment_len,
                   fragment);
    assert(len == full_len - 1);

    dest = url->buf + strlen(url->buf) + 1;

    scheme_len -= (scheme_len != 0);     // Skip ":"
    userinfo_len -= (userinfo_len != 0); // Skip "@"
    port_len -= (port_len != 0);         // Skip ":"
    port += (port_len != 0);             // Skip ":"
    query_len -= (query_len != 0);       // Skip "?"
    query += (query_len != 0);           // Skip "?"
    fragment_len -= (fragment_len != 0); // Skip "#"
    fragment += (fragment_len != 0);     // Skip "#"

    len = sprintf(dest, s2, scheme_len, scheme, 0, userinfo_len, userinfo, 0,
                  host_len, host, 0, port_len, port, 0, path_len, path, 0,
                  query_len, query, 0, fragment_len, fragment, 0);
    assert(len == parts_len - 1);

    url->str = url->buf;
    url->scheme = dest;
    url->userinfo = dest + scheme_len + 1;
    url->host = url->userinfo + userinfo_len + 1;
    url->port = url->host + host_len + 1;
    url->path = url->port + port_len + 1;
    url->query = url->path + path_len + 1;
    url->fragment = url->query + query_len + 1;

    return url;
}

void sc_url_destroy(struct sc_url *url)
{
    sc_url_free(url);
}
