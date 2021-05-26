/*
 * shellcall.h
 *
 *  Created on: May 17, 2021
 *      Author: 25412
 */

#ifndef LIBSYLIXOS_SYLIXOS_SHELL_INTERPRETER_SHELLCALL_H_
#define LIBSYLIXOS_SYLIXOS_SHELL_INTERPRETER_SHELLCALL_H_

#include <stddef.h>

enum shellcall_status {
    shellcall_failed,
    shellcall_success,
    shellcall_truncate
};

int shellcall(const char *cmd, char *out_buf, size_t buf_len);

#endif /* LIBSYLIXOS_SYLIXOS_SHELL_INTERPRETER_SHELLCALL_H_ */
