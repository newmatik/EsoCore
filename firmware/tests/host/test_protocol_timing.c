/**
 * @file test_protocol_timing.c
 * @brief Host unit tests for the receive timeout of common/communication/protocol.c
 *
 * protocol.c is compiled unchanged. Its RS-485 hardware layer is still a stub
 * that never receives a byte, which is what a silent bus looks like: every
 * receive must end after its timeout (with a clock) or after one read (without
 * one) instead of waiting forever. Built and run with `make test` in firmware/.
 */

#include "protocol.h"

#include <stdio.h>
#include <string.h>

static int checks_run = 0;
static int checks_failed = 0;
static const char *current_test = "";

#define CHECK(condition)                                                              \
    do {                                                                              \
        checks_run++;                                                                 \
        if (!(condition)) {                                                           \
            checks_failed++;                                                          \
            printf("FAIL %s (%s:%d): %s\n", current_test, __FILE__, __LINE__, #condition); \
        }                                                                             \
    } while (0)

/* A clock that advances by one millisecond each time it is read */
static uint32_t fake_now_ms = 0;

static uint32_t ticking_clock(void) {
    return fake_now_ms++;
}

static uint32_t error_count(void) {
    uint32_t errors = 0;
    CHECK(esocore_protocol_get_statistics(NULL, NULL, &errors));
    return errors;
}

static void test_receive_without_clock_returns(void) {
    current_test = "receive_without_clock_returns";
    esocore_protocol_set_time_source(NULL);
    CHECK(esocore_protocol_init(0x02, ESOCORE_DEVICE_TYPE_VIBRATION));
    esocore_message_t message;
    uint32_t before = error_count();
    CHECK(!esocore_protocol_receive_message(&message, 50));
    CHECK(!esocore_protocol_receive_message(&message, 0));
    CHECK(error_count() == before + 2);
    esocore_protocol_deinit();
}

static void test_receive_times_out_with_clock(void) {
    current_test = "receive_times_out_with_clock";
    fake_now_ms = 1000;
    esocore_protocol_set_time_source(ticking_clock);
    CHECK(esocore_protocol_init(0x00, ESOCORE_DEVICE_TYPE_MASTER));
    esocore_message_t message;
    uint32_t before = error_count();

    /* 50 ms: returns once the clock has moved past the timeout, not earlier */
    CHECK(!esocore_protocol_receive_message(&message, 50));
    CHECK(fake_now_ms > 1000 + 50);
    CHECK(fake_now_ms < 1000 + 200);
    CHECK(error_count() == before + 1);

    /* Timeout 0 (the Edge main loop): returns as soon as the clock moves */
    uint32_t start = fake_now_ms;
    CHECK(!esocore_protocol_receive_message(&message, 0));
    CHECK(fake_now_ms - start < 10);
    CHECK(error_count() == before + 2);

    esocore_protocol_set_time_source(NULL);
    esocore_protocol_deinit();
}

static void test_receive_times_out_across_wrap(void) {
    current_test = "receive_times_out_across_wrap";
    fake_now_ms = 0xFFFFFFF0U;
    esocore_protocol_set_time_source(ticking_clock);
    CHECK(esocore_protocol_init(0x00, ESOCORE_DEVICE_TYPE_MASTER));
    esocore_message_t message;
    CHECK(!esocore_protocol_receive_message(&message, 40));
    CHECK(fake_now_ms > 0x18U && fake_now_ms < 0x100U); /* 0xFFFFFFF0 + 41 wraps to 0x19 */
    esocore_protocol_set_time_source(NULL);
    esocore_protocol_deinit();
}

int main(void) {
    test_receive_without_clock_returns();
    test_receive_times_out_with_clock();
    test_receive_times_out_across_wrap();

    printf("protocol timing tests: %d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
