#ifndef NETMETER_UPLOAD_H
#define NETMETER_UPLOAD_H

#include "download.h"

/* Uploads a buffer of size_mb megabytes via POST. show_progress draws a bar on stderr.
 * Returns 0 on success and -1 on failure (the error is printed on stderr). */
int run_upload(int size_mb, int show_progress, transfer_result_t *result);

#endif
