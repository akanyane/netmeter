#define _POSIX_C_SOURCE 200809L

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

#include "download.h"
#include "ping.h"
#include "upload.h"
#include "utils.h"

#define DEFAULT_DOWNLOAD_MB 25
#define DEFAULT_UPLOAD_MB 10
#define MAX_SIZE_MB 1024

static void print_usage(const char *prog)
{
    printf("netmeter - measures your internet speed\n\n"
           "Usage: %s [options]\n\n"
           "Options:\n"
           "  -d, --download-only   run only the download test\n"
           "  -u, --upload-only     run only the upload test\n"
           "  -p, --ping-only       run only the latency (ping) test\n"
           "  -s, --size <MB>       test size in MB (default: %d MB download, %d MB upload)\n"
           "  -j, --json            output in JSON format\n"
           "  -h, --help            show this help\n",
           prog, DEFAULT_DOWNLOAD_MB, DEFAULT_UPLOAD_MB);
}

static int parse_size(const char *text, int *out)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (end == text || *end != '\0' || value < 1 || value > MAX_SIZE_MB)
        return -1;
    *out = (int)value;
    return 0;
}

int main(int argc, char **argv)
{
    static const struct option long_options[] = {
        {"download-only", no_argument, NULL, 'd'},
        {"upload-only", no_argument, NULL, 'u'},
        {"ping-only", no_argument, NULL, 'p'},
        {"size", required_argument, NULL, 's'},
        {"json", no_argument, NULL, 'j'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};

    int only_download = 0, only_upload = 0, only_ping = 0, json = 0;
    int size_mb = 0;
    int run_ping_test, run_download_test, run_upload_test;
    int show_progress, opt, status = EXIT_SUCCESS;
    ping_result_t ping = {0};
    transfer_result_t download = {0}, upload = {0};
    CURLcode rc;

    while ((opt = getopt_long(argc, argv, "dups:jh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'd':
            only_download = 1;
            break;
        case 'u':
            only_upload = 1;
            break;
        case 'p':
            only_ping = 1;
            break;
        case 's':
            if (parse_size(optarg, &size_mb) != 0) {
                fprintf(stderr, "Error: invalid size '%s' (use an integer between 1 and %d MB)\n",
                        optarg, MAX_SIZE_MB);
                return EXIT_FAILURE;
            }
            break;
        case 'j':
            json = 1;
            break;
        case 'h':
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "Use '%s --help' to see the available options.\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (only_download + only_upload + only_ping > 1) {
        fprintf(stderr, "Error: -d, -u and -p cannot be used together.\n");
        return EXIT_FAILURE;
    }

    run_ping_test = !only_download && !only_upload;
    run_download_test = !only_upload && !only_ping;
    run_upload_test = !only_download && !only_ping;
    show_progress = !json && isatty(fileno(stderr));

    rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK) {
        fprintf(stderr, "Error: could not initialize libcurl: %s\n", curl_easy_strerror(rc));
        return EXIT_FAILURE;
    }

    if (run_ping_test) {
        if (!json)
            fprintf(stderr, "Measuring latency...\n");
        if (run_ping(&ping) != 0) {
            status = EXIT_FAILURE;
            goto done;
        }
    }
    if (run_download_test && run_download(size_mb ? size_mb : DEFAULT_DOWNLOAD_MB,
                                          show_progress, &download) != 0) {
        status = EXIT_FAILURE;
        goto done;
    }
    if (run_upload_test && run_upload(size_mb ? size_mb : DEFAULT_UPLOAD_MB,
                                      show_progress, &upload) != 0) {
        status = EXIT_FAILURE;
        goto done;
    }

    if (json) {
        const char *sep = "";

        printf("{");
        if (run_ping_test) {
            printf("\"ping\":{\"min_ms\":%.2f,\"avg_ms\":%.2f,\"jitter_ms\":%.2f,\"samples\":%d}",
                   ping.min_ms, ping.avg_ms, ping.jitter_ms, ping.samples);
            sep = ",";
        }
        if (run_download_test) {
            printf("%s\"download\":{\"mbps\":%.2f,\"bytes\":%lld,\"seconds\":%.3f}", sep,
                   download.mbps, (long long)download.bytes, download.seconds);
            sep = ",";
        }
        if (run_upload_test)
            printf("%s\"upload\":{\"mbps\":%.2f,\"bytes\":%lld,\"seconds\":%.3f}", sep,
                   upload.mbps, (long long)upload.bytes, upload.seconds);
        printf("}\n");
    } else {
        printf("=== netmeter ===\n");
        if (run_ping_test)
            printf("Latency (%d connections)\n  Minimum : %8.2f ms\n  Average : %8.2f ms\n"
                   "  Jitter  : %8.2f ms\n",
                   ping.samples, ping.min_ms, ping.avg_ms, ping.jitter_ms);
        if (run_download_test)
            printf("Download : %8.2f Mbps  (%.1f MB in %.2f s)\n", download.mbps,
                   (double)download.bytes / (1024.0 * 1024.0), download.seconds);
        if (run_upload_test)
            printf("Upload   : %8.2f Mbps  (%.1f MB in %.2f s)\n", upload.mbps,
                   (double)upload.bytes / (1024.0 * 1024.0), upload.seconds);
    }

done:
    curl_global_cleanup();
    return status;
}
