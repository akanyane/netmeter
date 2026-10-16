#include "download.h"
#include "utils.h"

#include <stdio.h>

#define DOWNLOAD_URL "https://speed.cloudflare.com/__down?bytes=%lld"

int run_download(int size_mb, int show_progress, transfer_result_t *result)
{
    char errbuf[CURL_ERROR_SIZE];
    char url[128];
    progress_ctx_t progress;
    curl_off_t expected = (curl_off_t)size_mb * 1024 * 1024;
    curl_off_t total_us = 0, pretransfer_us = 0, downloaded = 0;
    long status = 0;
    CURLcode rc;
    CURL *curl = create_handle(errbuf);

    if (!curl) {
        fprintf(stderr, "Erro no teste de download: não foi possível inicializar a libcurl\n");
        return -1;
    }

    snprintf(url, sizeof(url), DOWNLOAD_URL, (long long)expected);
    progress_init(&progress, "Download", 0, expected, show_progress);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);

    rc = curl_easy_perform(curl);
    progress_finish(&progress);
    if (rc != CURLE_OK) {
        print_curl_error("download", rc, errbuf);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &downloaded);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_us);
    curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME_T, &pretransfer_us);
    curl_easy_cleanup(curl);

    if (status != 200) {
        print_http_error("download", status);
        return -1;
    }

    /* Exclude DNS, TCP and TLS setup so only the data transfer is measured. */
    total_us -= pretransfer_us;
    result->bytes = downloaded;
    result->seconds = (double)total_us / 1e6;
    result->mbps = bytes_to_mbps(downloaded, total_us);
    return 0;
}
