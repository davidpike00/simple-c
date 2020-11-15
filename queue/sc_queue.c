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

#include "sc_queue.h"

#include <stdbool.h>

#ifndef SC_SIZE_MAX
    #define SC_SIZE_MAX SIZE_MAX
#endif

#define SC_MAX_CAP ((SC_SIZE_MAX - sizeof(struct sc_queue)) / 2ul)

static const struct sc_queue sc_empty = {.cap = 1, .first = 0, .last = 0};

static void *queue_alloc(void *prev, size_t elem_size, size_t *cap)
{
    size_t alloc;
    size_t v = *cap;
    void *t;

    if (*cap > SC_MAX_CAP) {
        sc_queue_on_error("Max capacity has been exceed. cap(%zu). ", (*cap));
        return NULL;
    }

    // Find next power of two.
    v = v < 4 ? 4 : v;
    v--;
    for (size_t i = 1; i < sizeof(v) * 8; i *= 2) {
        v |= v >> i;
    }
    v++;

    alloc = sizeof(struct sc_queue) + (elem_size * v);
    t = sc_queue_realloc(prev, alloc);
    if (t == NULL) {
        sc_queue_on_error("Out of memory. alloc(%zu). ", alloc);
    }

    *cap = v;

    return t;
}

bool sc_queue_init(void **q, size_t elem_size, size_t cap)
{
    size_t p = cap;
    struct sc_queue *meta;

    if (cap == 0) {
        *q = (void *) sc_empty.elems;
        return true;
    }

    meta = queue_alloc(NULL, elem_size, &p);
    if (meta == NULL) {
        *q = NULL;
        return false;
    }

    meta->cap = p;
    meta->first = 0;
    meta->last = 0;
    *q = meta->elems;

    return true;
}

void sc_queue_term(void **q)
{
    struct sc_queue *meta = sc_queue_meta(*q);

    if (meta != &sc_empty) {
        sc_queue_free(meta);
    }

    *q = NULL;
}

bool sc_queue_expand(void **q, size_t elem_size)
{
    struct sc_queue *tmp;
    struct sc_queue *meta = sc_queue_meta(*q);
    size_t cap, count, size;
    size_t pos = (meta->last + 1) & (meta->cap - 1);
    uint8_t *e;

    if (pos == meta->first) {
        if (meta == &sc_empty) {
            return sc_queue_init(q, elem_size, 4);
        }

        cap = meta->cap * 2;
        tmp = queue_alloc(meta, elem_size, &cap);
        if (tmp == NULL) {
            return false;
        }

        /**
         * Move items to make empty slots at the end.
         * e.g :
         *               last    first
         *                |       |
         * Step 0 : | 2 | 3 | - | 1 |                  // tmp->cap : 4
         * Step 1 : | 2 | 3 | - | 1 | - | - | - | - |  // realloc
         * Step 2 : | 2 | 3 | - | 1 | 1 | - | - | - |  // mempcy
         * Step 3 : | 2 | 2 | 3 | 1 | 1 | - | - | - |  // memmove
         * Step 4 : | 1 | 2 | 3 | 1 | 1 | - | - | - |  // mempcy
         * Step 5 : | 1 | 2 | 3 | - | - | - | - | - |  // tmp->last = cap - 1;
         *           |       |
         *         first   last
         *
         */

        e = tmp->elems;
        count = tmp->cap - tmp->first;
        size = elem_size;

        memcpy(e + (size * tmp->cap), e + (size * tmp->first), count * size);
        memmove(e + (count * size), e, tmp->first * size);
        memcpy(e, e + (size * tmp->cap), count * size);

        tmp->last = tmp->cap - 1;
        tmp->first = 0;
        tmp->cap = cap;
        *q = tmp->elems;
    }

    return true;
}
