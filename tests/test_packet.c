/*
 * test_packet.c - unit tests for the packet module.
 * SPDX-License-Identifier: MIT
 */

#include "packet.h"
#include "test_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int pkt_create_basic(void) {
    const uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    packet_t *pkt = packet_create(42, 7, payload, sizeof(payload), "hello");
    TEST_ASSERT(pkt != NULL, "packet_create returned NULL");
    TEST_ASSERT_EQ_INT(pkt->id, 42, "id mismatch");
    TEST_ASSERT_EQ_INT(pkt->type, 7, "type mismatch");
    TEST_ASSERT_EQ_INT(pkt->length, sizeof(payload), "length mismatch");
    TEST_ASSERT(pkt->payload != NULL, "payload allocated");
    TEST_ASSERT(memcmp(pkt->payload, payload, sizeof(payload)) == 0,
                "payload bytes mismatch");
    TEST_ASSERT_EQ_STR(pkt->tag, "hello", "tag string mismatch");
    packet_free(pkt);
    return 0;
}

static int pkt_create_deep_copies_payload(void) {
    uint8_t payload[] = {1, 2, 3};
    packet_t *pkt = packet_create(1, 0, payload, sizeof(payload), "tag");
    TEST_ASSERT(pkt != NULL, "packet_create returned NULL");

    /* Mutating the caller's buffer must not affect the packet. */
    payload[0] = 99;
    TEST_ASSERT_EQ_INT(pkt->payload[0], 1, "payload should be deep-copied");

    packet_free(pkt);
    return 0;
}

static int pkt_create_deep_copies_tag(void) {
    char tag[] = "mytag";
    packet_t *pkt = packet_create(2, 0, (uint8_t *)"x", 1, tag);
    TEST_ASSERT(pkt != NULL, "packet_create returned NULL");

    tag[0] = 'Z';
    TEST_ASSERT_EQ_STR(pkt->tag, "mytag", "tag should be deep-copied");

    packet_free(pkt);
    return 0;
}

static int pkt_free_null_is_noop(void) {
    packet_free(NULL);
    return 0;
}

/* --- packet_clone tests --- */

static int pkt_clone_payload_is_independent(void) {
    const uint8_t data[] = {0x01, 0x02, 0x03};
    packet_t *src = packet_create(10, 1, data, sizeof(data), "tag");
    TEST_ASSERT(src != NULL, "src packet_create returned NULL");

    packet_t *dst = packet_clone(src);
    TEST_ASSERT(dst != NULL, "packet_clone returned NULL");

    /* Pointers must differ — distinct allocations. */
    TEST_ASSERT(dst->payload != src->payload, "payload should be a distinct allocation");

    /* Mutate src payload; dst must be unaffected. */
    src->payload[0] = 0xFF;
    TEST_ASSERT_EQ_INT(dst->payload[0], 0x01, "clone payload should be independent");

    packet_free(src);
    packet_free(dst);
    return 0;
}

static int pkt_clone_tag_is_independent(void) {
    const uint8_t data[] = {0xAA};
    packet_t *src = packet_create(11, 1, data, sizeof(data), "original");
    TEST_ASSERT(src != NULL, "src packet_create returned NULL");

    packet_t *dst = packet_clone(src);
    TEST_ASSERT(dst != NULL, "packet_clone returned NULL");

    /* Pointers must differ — distinct heap allocations. */
    TEST_ASSERT(dst->tag != src->tag, "tag should be a distinct allocation");

    /* Free src tag; dst->tag must remain valid and unchanged. */
    free(src->tag);
    src->tag = NULL;
    TEST_ASSERT(dst->tag != NULL, "dst->tag should be unaffected after freeing src->tag");
    TEST_ASSERT_EQ_STR(dst->tag, "original", "dst->tag content should be unchanged");

    packet_free(dst);
    /* src->tag already freed above; avoid double-free by clearing before packet_free. */
    packet_free(src);
    return 0;
}

static int pkt_clone_null_returns_null(void) {
    TEST_ASSERT(packet_clone(NULL) == NULL, "clone of NULL should return NULL");
    return 0;
}

static int pkt_clone_zero_length(void) {
    packet_t *src = packet_create(20, 0, NULL, 0, NULL);
    TEST_ASSERT(src != NULL, "packet_create(length=0) returned NULL");

    packet_t *dst = packet_clone(src);
    TEST_ASSERT(dst != NULL, "clone of zero-length packet returned NULL");
    TEST_ASSERT_EQ_INT(dst->length, 0, "cloned length should be 0");

    packet_free(src);
    packet_free(dst);
    return 0;
}

static int pkt_clone_null_tag(void) {
    const uint8_t data[] = {0x55};
    packet_t *src = packet_create(30, 0, data, sizeof(data), NULL);
    TEST_ASSERT(src != NULL, "packet_create(tag=NULL) returned NULL");

    packet_t *dst = packet_clone(src);
    TEST_ASSERT(dst != NULL, "clone with NULL tag returned NULL");
    TEST_ASSERT(dst->tag == NULL, "cloned tag should be NULL");

    packet_free(src);
    packet_free(dst);
    return 0;
}

int test_packet_run(void) {
    int failures = 0;
    printf("[packet]\n");
    TEST_RUN(pkt_create_basic);
    TEST_RUN(pkt_create_deep_copies_payload);
    TEST_RUN(pkt_create_deep_copies_tag);
    TEST_RUN(pkt_free_null_is_noop);
    TEST_RUN(pkt_clone_payload_is_independent);
    TEST_RUN(pkt_clone_tag_is_independent);
    TEST_RUN(pkt_clone_null_returns_null);
    TEST_RUN(pkt_clone_zero_length);
    TEST_RUN(pkt_clone_null_tag);
    return failures;
}
