#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "cbuffer.h"

void TestCbuffer_set_size_with_init(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536u);
    CuAssertTrue(tc, 65536u == cbuf_size(&cb));
}

void TestCbuffer_is_empty_after_init(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    CuAssertTrue(tc, cbuf_is_empty(&cb));
}

void TestCbuffer_is_not_empty_after_offer(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
    CuAssertTrue(tc, !cbuf_is_empty(&cb));
}

void TestCbuffer_is_empty_after_poll_release(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
    cbuf_poll(&cb, 4);
    CuAssertTrue(tc, cbuf_is_empty(&cb));
}

void TestCbuffer_spaceused_is_zero_after_poll_release(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
    CuAssertTrue(tc, 4 == cbuf_usedspace(&cb));
    cbuf_poll(&cb, 4);
    CuAssertTrue(tc, 0 == cbuf_usedspace(&cb));
}

void TestCbuffer_cant_offer_if_not_enough_space(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    CuAssertTrue(tc, 0 == cbuf_offer(&cb, (unsigned char*)"1000", 1 << 17));
}

/**
 * We don't want to be full because the count() function will break down */
void TestCbuffer_cant_offer_if_buffer_will_be_completely_full(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    CuAssertTrue(tc, 0 == cbuf_offer(&cb, (unsigned char*)"1000", 1 << 16));
}

void TestCbuffer_offer_and_poll(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
//    CuAssertTrue(tc, 96 == cbuf_get_unused_size(&cb));
    CuAssertTrue(tc, 0 == strncmp("abcd", (char*)cbuf_poll(&cb, 4), 4));
}

void TestCbuffer_cant_poll_nonexistant(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    CuAssertTrue(tc, NULL == cbuf_poll(&cb, 4));
}

#if 0
void TxestCbuffer_can_poll_twice_without_release(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);

    cbuf_offer(&cb, (unsigned char*)"1000", 4);
    cbuf_poll(&cb, 4);
    CuAssertTrue(tc, NULL != cbuf_poll(&cb, 4));
}
#endif

void TestCbuffer_cant_poll_twice_when_released(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_offer(&cb, (unsigned char*)"1000", 4);
    cbuf_poll(&cb, 4);
    cbuf_poll(&cb, 4);
    CuAssertTrue(tc, NULL == cbuf_poll(&cb, 4));
}

void TestCbuffer_cbuffers_independant_of_each_other(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_t cb2;
    cbuf_new(&cb2, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
    cbuf_offer(&cb2, (unsigned char*)"efgh", 4);
    CuAssertTrue(tc, 0 == strncmp("abcd", (char*)cbuf_poll(&cb, 4), 4));
    CuAssertTrue(tc, 0 == strncmp("efgh", (char*)cbuf_poll(&cb2, 4), 4));
}

void TestCbuffer_cbuffers_independant_of_each_other_with_no_polling(CuTest * tc)
{
    cbuf_t cb;
    cbuf_new(&cb, 65536);
    cbuf_t cb2;
    cbuf_new(&cb2, 65536);
    cbuf_offer(&cb, (unsigned char*)"abcd", 4);
    cbuf_offer(&cb2, (unsigned char*)"efgh", 4);
    CuAssertTrue(tc, 0 == strncmp("abcd", (char*)cbuf_peek(&cb), 4));
    CuAssertTrue(tc, 0 == strncmp("efgh", (char*)cbuf_peek(&cb2), 4));
}

#if 0
void TxestCbuffer_get_unused_when_overlapping(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, (unsigned char*)"123", 3);
    cbuf_poll(cb, 2);
    cbuf_offer(cb, (unsigned char*)"45", 2);
    CuAssertTrue(tc, 0 == strncmp("1000", (char*)cbuf_poll(cb, 4), 4));
}
#endif
