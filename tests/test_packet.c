/*
 * test_packet.c - unit tests for the packet module.
 * SPDX-License-Identifier: MIT
 */

#include "packet.h"
#include "test_helpers.h"

#include <stdio.h>
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

static int pkt_clone_is_deep_copy(void) {
    uint8_t data[] = {0x01, 0x02, 0x03};
    packet_t *src = packet_create(1, 0, data, sizeof(data), "test");
    TEST_ASSERT(src != NULL, "src packet_create returned NULL");

    packet_t *dst = packet_clone(src);
    TEST_ASSERT(dst != NULL, "packet_clone returned NULL");

    dst->payload[0] = 0xFF;
    TEST_ASSERT_EQ_INT(src->payload[0], 0x01,
                       "clone must not share payload");

    dst->tag[0] = 'X';
    TEST_ASSERT_EQ_INT(src->tag[0], 't',
                       "clone must not share tag");

    packet_free(src);
    packet_free(dst);

    return 0;
}

static int pkt_clone_null_returns_null(void) {
    TEST_ASSERT(packet_clone(NULL) == NULL,
                "packet_clone(NULL) must return NULL");
    return 0;
}

static int pkt_clone_zero_length(void) {
    packet_t *src =
        packet_create(1, 0, NULL, 0, "tag");

    TEST_ASSERT(src != NULL,
                "packet_create failed");

    packet_t *dst = packet_clone(src);

    TEST_ASSERT(dst != NULL,
                "clone failed");

    TEST_ASSERT_EQ_INT(dst->length, 0,
                       "length should be zero");

    return 0;
}

static int pkt_clone_null_tag(void) {
    uint8_t data[] = {0x01};

    packet_t *src =
        packet_create(1, 0, data, sizeof(data), NULL);

    TEST_ASSERT(src != NULL, "packet_create failed");

    packet_t *dst = packet_clone(src);

    TEST_ASSERT(dst != NULL,
                "clone failed with NULL tag");

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
    TEST_RUN(pkt_clone_is_deep_copy);
    TEST_RUN(pkt_clone_null_returns_null);
    TEST_RUN(pkt_clone_zero_length);
    TEST_RUN(pkt_clone_null_tag);
    return failures;
}
