#ifndef NETMETER_DOWNLOAD_H
#define NETMETER_DOWNLOAD_H

#include <curl/curl.h>

typedef struct {
    double mbps;
    curl_off_t bytes;
    double seconds;
} transfer_result_t;

/* Downloads size_mb megabytes and discards them. show_progress draws a bar on stderr.
 * Returns 0 on success and -1 on failure (the error is printed on stderr). */
int run_download(int size_mb, int show_progress, transfer_result_t *result);

#endif
