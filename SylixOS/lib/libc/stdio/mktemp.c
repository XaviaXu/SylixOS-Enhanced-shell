/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "stdio.h"

#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "errno.h"
#include "stdio.h"
#include "ctype.h"
#include "unistd.h"

static int _gettemp(char *, int *, int, int);

static const char padchar[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int
mkstemps(path, slen)
    char *path;
    int slen;
{
    int fd;

    return (_gettemp(path, &fd, 0, slen) ? fd : -1);
}

int
mkstemp(path)
    char *path;
{
    int fd;

    return (_gettemp(path, &fd, 0, 0) ? fd : -1);
}

char *
mkdtemp(path)
    char *path;
{
    return (_gettemp(path, (int *)NULL, 1, 0) ? path : (char *)NULL);
}

char *
_mktemp(path)
    char *path;
{
    return (_gettemp(path, (int *)NULL, 0, 0) ? path : (char *)NULL);
}

char *
mktemp(path)
    char *path;
{
    return (_mktemp(path));
}

static int
_gettemp(path, doopen, domkdir, slen)
    char *path;
    int *doopen;
    int domkdir;
    int slen;
{
    char *start, *trv, *suffp, *carryp;
    char *pad;
    struct stat sbuf;
    int rval;
    uint32_t rand;
    char carrybuf[MAXPATHLEN];
    
    if ((doopen != NULL && domkdir) || slen < 0) {
        errno = EINVAL;
        return (0);
    }

    for (trv = path; *trv != '\0'; ++trv) {
        ;
    }
        
    if (trv - path >= MAXPATHLEN) {
        errno = ENAMETOOLONG;
        return (0);
    }
    trv -= slen;
    suffp = trv;
    --trv;
    if (trv < path || NULL != lib_strchr(suffp, '/')) {
        errno = EINVAL;
        return (0);
    }

    /* Fill space with random characters */
    while (trv >= path && *trv == 'X') {
        rand = lib_rand() % (sizeof(padchar) - 1);
        *trv-- = padchar[rand];
    }
    start = trv + 1;

    /* save first combination of random characters */
    lib_memcpy(carrybuf, start, suffp - start);

    /*
     * check the target directory.
     */
    if (doopen != NULL || domkdir) {
        for (; trv > path; --trv) {
            if (*trv == '/') {
                *trv = '\0';
                rval = stat(path, &sbuf);
                *trv = '/';
                if (rval != 0)
                        return (0);
                if (!S_ISDIR(sbuf.st_mode)) {
                    errno = ENOTDIR;
                    return (0);
                }
                break;
            }
        }
    }

    for (;;) {
        if (doopen) {
            if ((*doopen =
                open(path, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
                    return (1);
            if (errno != EEXIST)
                    return (0);
        } else if (domkdir) {
            if (mkdir(path, 0700) == 0)
                    return (1);
            if (errno != EEXIST)
                    return (0);
        } else if (lstat(path, &sbuf))
            return (errno == ENOENT);

        /* If we have a collision, cycle through the space of filenames */
        for (trv = start, carryp = carrybuf;;) {
            /* have we tried all possible permutations? */
            if (trv == suffp)
                return (0); /* yes - exit with EEXIST */
            pad = lib_strchr(padchar, *trv);
            if (pad == NULL) {
                /* this should never happen */
                errno = EIO;
                return (0);
            }
            /* increment character */
            *trv = (*++pad == '\0') ? padchar[0] : *pad;
            /* carry to next position? */
            if (*trv == *carryp) {
                /* increment position and loop */
                ++trv;
                ++carryp;
            } else {
                /* try with new name */
                break;
            }
        }
    }
    /*NOTREACHED*/
}

#endif  /*  (LW_CFG_DEVICE_EN > 0)      */
        /*  (LW_CFG_FIO_LIB_EN > 0)     */
