#ifndef NETMETER_UTILS_H
#define NETMETER_UTILS_H

#include <stddef.h>
#include <curl/curl.h>

#define NETMETER_USER_AGENT "netmeter/1.0"

/* State shared with the progress callback of a single transfer. */
typedef struct {
    const char *label;       /* text shown before the bar */
    int upload;              /* non-zero: track upload bytes, otherwise download */
    int enabled;             /* non-zero: draw the bar */
    curl_off_t expected;     /* fallback total when libcurl does not know it yet */
    int last_percent;        /* last percentage drawn, avoids redundant redraws */
} progress_ctx_t;

/* Creates an easy handle with the common options. errbuf must hold CURL_ERROR_SIZE bytes
 * and outlive the handle. Returns NULL on failure. */
CURL *create_handle(char *errbuf);

/* Write callback that throws the received data away. */
size_t discard_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

/* CURLOPT_XFERINFOFUNCTION callback: draws a progress bar on stderr. */
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow);

void progress_init(progress_ctx_t *ctx, const char *label, int upload,
                   curl_off_t expected, int enabled);

/* Erases the progress bar line. */
void progress_finish(const progress_ctx_t *ctx);

/* Prints a clear message (in Portuguese) for a libcurl error on stderr. */
void print_curl_error(const char *context, CURLcode code, const char *errbuf);

/* Prints a message for an unexpected HTTP status on stderr. */
void print_http_error(const char *context, long status);

/* Converts bytes transferred over a time in microseconds to megabits per second. */
double bytes_to_mbps(curl_off_t bytes, curl_off_t micros);

#endif
