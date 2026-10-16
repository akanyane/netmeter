#ifndef NETMETER_PING_H
#define NETMETER_PING_H

#define PING_SAMPLES 5

typedef struct {
    double min_ms;
    double avg_ms;
    double jitter_ms;   /* mean absolute difference between consecutive samples */
    int samples;
} ping_result_t;

/* Computes min, average and jitter from n handshake samples in milliseconds (n >= 1).
 * Does not touch the network. */
void ping_compute_stats(const double *samples, int n, ping_result_t *result);

/* Opens PING_SAMPLES fresh connections and measures the TCP handshake time of each.
 * Returns 0 on success and -1 on failure (the error is printed on stderr). */
int run_ping(ping_result_t *result);

#endif
