#include "upload.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UPLOAD_URL "https://speed.cloudflare.com/__up"

int run_upload(int size_mb, int show_progress, transfer_result_t *result)
{
    char errbuf[CURL_ERROR_SIZE];
    progress_ctx_t progress;
    struct curl_slist *headers = NULL;
    curl_off_t size = (curl_off_t)size_mb * 1024 * 1024;
    curl_off_t total_us = 0, pretransfer_us = 0, uploaded = 0;
    long status = 0;
    CURLcode rc;
    CURL *curl;
    char *buffer;

    buffer = malloc((size_t)size);
    if (!buffer) {
        fprintf(stderr, "Erro no teste de upload: memória insuficiente para alocar %d MB\n", size_mb);
        return -1;
    }
    memset(buffer, 'x', (size_t)size);

    curl = create_handle(errbuf);
    if (!curl) {
        fprintf(stderr, "Erro no teste de upload: não foi possível inicializar a libcurl\n");
        free(buffer);
        return -1;
    }

    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    progress_init(&progress, "Upload", 1, size, show_progress);

    curl_easy_setopt(curl, CURLOPT_URL, UPLOAD_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);

    rc = curl_easy_perform(curl);
    progress_finish(&progress);
    if (rc == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &uploaded);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_us);
        curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME_T, &pretransfer_us);
    } else {
        print_curl_error("upload", rc, errbuf);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(buffer);

    if (rc != CURLE_OK)
        return -1;
    if (status != 200) {
        print_http_error("upload", status);
        return -1;
    }

    /* Exclude DNS, TCP and TLS setup so only the data transfer is measured. */
    total_us -= pretransfer_us;
    result->bytes = uploaded;
    result->seconds = (double)total_us / 1e6;
    result->mbps = bytes_to_mbps(uploaded, total_us);
    return 0;
}
