#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ping.h"
#include "utils.h"

static int failures = 0;

#define CHECK_NEAR(actual, expected)                                                    \
    do {                                                                                \
        double a_ = (actual), e_ = (expected);                                          \
        if (fabs(a_ - e_) > 1e-9) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s = %f, expected %f\n", __FILE__, __LINE__,   \
                    #actual, a_, e_);                                                   \
            failures++;                                                                 \
        }                                                                               \
    } while (0)

static void test_bytes_to_mbps(void)
{
    /* 125000 bytes = 1 Mbit in 1 second */
    CHECK_NEAR(bytes_to_mbps(125000, 1000000), 1.0);
    /* 25 MB in 2 seconds */
    CHECK_NEAR(bytes_to_mbps(25 * 1024 * 1024, 2000000), 25.0 * 1024 * 1024 * 8 / 2 / 1e6);
    /* invalid elapsed time must not divide by zero */
    CHECK_NEAR(bytes_to_mbps(1000, 0), 0.0);
    CHECK_NEAR(bytes_to_mbps(1000, -5), 0.0);
}

static void test_ping_stats(void)
{
    const double varied[] = {10.0, 12.0, 8.0, 11.0, 9.0};
    const double equal[] = {5.0, 5.0, 5.0, 5.0, 5.0};
    const double single[] = {7.5};
    ping_result_t r;

    ping_compute_stats(varied, 5, &r);
    CHECK_NEAR(r.min_ms, 8.0);
    CHECK_NEAR(r.avg_ms, 10.0);
    /* |12-10| + |8-12| + |11-8| + |9-11| = 2 + 4 + 3 + 2 = 11, divided by 4 */
    CHECK_NEAR(r.jitter_ms, 2.75);
    CHECK_NEAR(r.samples, 5);

    ping_compute_stats(equal, 5, &r);
    CHECK_NEAR(r.min_ms, 5.0);
    CHECK_NEAR(r.avg_ms, 5.0);
    CHECK_NEAR(r.jitter_ms, 0.0);

    ping_compute_stats(single, 1, &r);
    CHECK_NEAR(r.min_ms, 7.5);
    CHECK_NEAR(r.jitter_ms, 0.0);
}

int main(void)
{
    test_bytes_to_mbps();
    test_ping_stats();

    if (failures) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    printf("All tests passed\n");
    return EXIT_SUCCESS;
}
