
/**
 * Copyright (c) 2014, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cbuffer.h>

#define fail() assert(0)

/** OSX needs some help here */
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

static void __create_buffer_mirror(cbuf_t* cb)
{
    char path[] = "/tmp/cb-XXXXXX";

    int fd = mkstemp(path);
    assert(fd >= 0 && "cbuffer: mkstemp failed\n");

    int status = unlink(path);
    assert(status == 0 && "cbuffer: Unlink failed");

    status = ftruncate(fd, cb->size);
    assert(status == 0 && "cbuffer: ftruncate failed\n");

    /* create the array of data */
    cb->data = mmap(NULL, cb->size << 1, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE,
                    -1, 0);
    assert(cb->data != MAP_FAILED && "cbuffer: mmap(1) failed\n");

    void* address = mmap(cb->data, cb->size, PROT_READ | PROT_WRITE,
                         MAP_FIXED | MAP_SHARED, fd, 0);
    assert(address == cb->data && "cbuffer: mmap(2) failed\n");

    address = mmap(cb->data + cb->size, cb->size, PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_SHARED, fd, 0);
    assert((address == cb->data + cb->size) && "cbuffer: mmap(3) failed\n");

    status = close(fd);
    assert(status == 0 && "cbuffer: close failed\n");
}

void cbuf_new(cbuf_t* dst, const unsigned long int size)
{
    dst->size = size;
    dst->head = dst->tail = 0;
    __create_buffer_mirror(dst);
}

void cbuf_free(cbuf_t *me)
{
    munmap(me->data, me->size << 1);
    free(me);
}

int cbuf_is_empty(const cbuf_t *me)
{
    return me->head == me->tail;
}

int cbuf_offer(cbuf_t *me, const unsigned char *data, const int size)
{
    /* prevent buffer from getting completely full or over commited */
    if (cbuf_unusedspace(me) <= size)
        return 0;

    int written = cbuf_unusedspace(me);
    written = size < written ? size : written;
    memcpy(me->data + me->tail, data, written);
    me->tail += written;
    if (me->size < me->tail)
        me->tail %= me->size;
    return written;
}

unsigned char *cbuf_peek(const cbuf_t *me)
{
    if (cbuf_is_empty(me))
        return NULL;

    return me->data + me->head;
}

unsigned char *cbuf_poll(cbuf_t *me, const unsigned int size)
{
    if (cbuf_is_empty(me))
        return NULL;

    void *end = me->data + me->head;
    me->head += size;
    if (me->head >= me->size)
        me->head -= me->size;
    return end;
}

int cbuf_size(const cbuf_t *me)
{
    return me->size;
}

int cbuf_usedspace(const cbuf_t *me)
{
    if (me->head <= me->tail)
        return me->tail - me->head;
    else
        return me->size - (me->head - me->tail);
}

int cbuf_unusedspace(const cbuf_t *me)
{
    return me->size - cbuf_usedspace(me);
}
