#include "utils.h"

#include <stdio.h>
#include <string.h>

#define BAR_WIDTH 30

CURL *create_handle(char *errbuf)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return NULL;

    errbuf[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, NETMETER_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    return curl;
}

size_t discard_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    (void)ptr;
    (void)userdata;
    return size * nmemb;
}

void progress_init(progress_ctx_t *ctx, const char *label, int upload,
                   curl_off_t expected, int enabled)
{
    ctx->label = label;
    ctx->upload = upload;
    ctx->expected = expected;
    ctx->enabled = enabled;
    ctx->last_percent = -1;
}

int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow)
{
    progress_ctx_t *ctx = clientp;
    curl_off_t total, now;
    int percent, filled, i;

    if (!ctx || !ctx->enabled)
        return 0;

    total = ctx->upload ? ultotal : dltotal;
    now = ctx->upload ? ulnow : dlnow;
    if (total <= 0)
        total = ctx->expected;
    if (total <= 0)
        return 0;

    percent = (int)((now * 100) / total);
    if (percent > 100)
        percent = 100;
    if (percent < 0)
        percent = 0;
    if (percent == ctx->last_percent)
        return 0;
    ctx->last_percent = percent;

    filled = percent * BAR_WIDTH / 100;
    fprintf(stderr, "\r  %-9s [", ctx->label);
    for (i = 0; i < BAR_WIDTH; i++)
        fputc(i < filled ? '#' : '.', stderr);
    fprintf(stderr, "] %3d%%", percent);
    fflush(stderr);
    return 0;
}

void progress_finish(const progress_ctx_t *ctx)
{
    if (!ctx->enabled)
        return;
    fprintf(stderr, "\r%*s\r", BAR_WIDTH + 25, "");
    fflush(stderr);
}

void print_curl_error(const char *context, CURLcode code, const char *errbuf)
{
    const char *reason;

    switch (code) {
    case CURLE_COULDNT_RESOLVE_HOST:
        reason = "could not resolve the server address (check your connection and DNS)";
        break;
    case CURLE_COULDNT_RESOLVE_PROXY:
        reason = "could not resolve the proxy address";
        break;
    case CURLE_COULDNT_CONNECT:
        reason = "could not connect to the server";
        break;
    case CURLE_OPERATION_TIMEDOUT:
        reason = "timed out (the connection is too slow or unstable)";
        break;
    case CURLE_SSL_CONNECT_ERROR:
        reason = "TLS/SSL handshake failed";
        break;
    case CURLE_PEER_FAILED_VERIFICATION:
        reason = "failed to verify the server certificate";
        break;
    case CURLE_SEND_ERROR:
        reason = "failed to send data to the server";
        break;
    case CURLE_RECV_ERROR:
        reason = "failed to receive data from the server";
        break;
    case CURLE_GOT_NOTHING:
        reason = "the server closed the connection without sending a response";
        break;
    case CURLE_ABORTED_BY_CALLBACK:
        reason = "transfer aborted";
        break;
    case CURLE_OUT_OF_MEMORY:
        reason = "not enough memory";
        break;
    case CURLE_UNSUPPORTED_PROTOCOL:
        reason = "protocol not supported by this libcurl build";
        break;
    default:
        reason = curl_easy_strerror(code);
        break;
    }

    fprintf(stderr, "Error in %s test: %s (libcurl code %d)", context, reason, (int)code);
    if (errbuf && errbuf[0] != '\0' && code != CURLE_ABORTED_BY_CALLBACK)
        fprintf(stderr, " - detail: %s", errbuf);
    fputc('\n', stderr);
}

void print_http_error(const char *context, long status)
{
    fprintf(stderr, "Error in %s test: server responded with HTTP %ld (expected 200)\n",
            context, status);
}

double bytes_to_mbps(curl_off_t bytes, curl_off_t micros)
{
    if (micros <= 0)
        return 0.0;
    return ((double)bytes * 8.0) / ((double)micros / 1e6) / 1e6;
}
