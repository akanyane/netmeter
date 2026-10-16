#include "ping.h"
#include "utils.h"

#include <math.h>
#include <stdio.h>

#define PING_URL "https://speed.cloudflare.com/__down?bytes=0"

/* Measures one handshake on a brand new connection, in milliseconds. */
static int measure_handshake(double *out_ms)
{
    char errbuf[CURL_ERROR_SIZE];
    curl_off_t connect_us = 0, lookup_us = 0;
    long status = 0;
    CURLcode rc;
    CURL *curl = create_handle(errbuf);

    if (!curl) {
        fprintf(stderr, "Erro no teste de latência: não foi possível inicializar a libcurl\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, PING_URL);
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);

    rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        print_curl_error("latência", rc, errbuf);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    rc = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connect_us);
    if (rc == CURLE_OK)
        rc = curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME_T, &lookup_us);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        print_curl_error("latência", rc, NULL);
        return -1;
    }
    if (status != 200) {
        print_http_error("latência", status);
        return -1;
    }

    *out_ms = (double)(connect_us - lookup_us) / 1000.0;
    return 0;
}

int run_ping(ping_result_t *result)
{
    double samples[PING_SAMPLES];
    double sum = 0.0, diff_sum = 0.0;
    int i;

    for (i = 0; i < PING_SAMPLES; i++) {
        if (measure_handshake(&samples[i]) != 0)
            return -1;
    }

    result->min_ms = samples[0];
    for (i = 0; i < PING_SAMPLES; i++) {
        sum += samples[i];
        if (samples[i] < result->min_ms)
            result->min_ms = samples[i];
        if (i > 0)
            diff_sum += fabs(samples[i] - samples[i - 1]);
    }

    result->samples = PING_SAMPLES;
    result->avg_ms = sum / PING_SAMPLES;
    result->jitter_ms = diff_sum / (PING_SAMPLES - 1);
    return 0;
}
