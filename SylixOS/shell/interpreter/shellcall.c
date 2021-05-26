/*
 * shellcall.c
 *
 *  Created on: May 17, 2021
 *      Author: 25412
 */

#include <stdio.h>
#include "shellcall.h"

int shellcall(const char *cmd, char *out_buf, size_t buf_len) {
    FILE *out_file = popen(cmd, "r");
    if (out_file == NULL) {
        return shellcall_failed;
    }
    size_t cur = 0;
    while (cur < buf_len-1) {
        out_buf[cur] = fgetc(out_file);
        if (out_buf[cur] == EOF) {
            break;
        }
        ++cur;
    }
    out_buf[cur] = 0;
    if (cur == buf_len-1) {
        return shellcall_truncate;
    }
    else {
        return shellcall_success;
    }
}
