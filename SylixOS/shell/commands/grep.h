#define TNRM  "\x1B[0m"
#define TRED  "\033[1;31m"
#define TGRN  "\x1B[32m"
#define TYEL  "\x1B[33m"
#define TBLU  "\x1B[34m"
#define TMAG  "\x1B[35m"
#define TCYN  "\x1B[36m"
#define TWHT  "\x1B[37m"
#define TRSE  "\33[2K\r"
#define _GNU_SOURCE/*For strcasestr*/
#define UNUSED(x) (void)(x)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#ifndef LIBSYLIXOS_SYLIXOS_SHELL_COMMANDS_GREP_H_
#define LIBSYLIXOS_SYLIXOS_SHELL_COMMANDS_GREP_H_


int exec_find (int argc, char ** argv);

extern int cflag, vflag, iflag, wflag, Hflag, fflag, rflag, hflag, mflag, noflag, mnum, bflag, fileflag, eflag, qflag;
extern int opterr, nl;

typedef struct pat_info{
	char *pat;
	int *pos;
}pat_info;

int check(char *token, const char *substr);

int valid(char ch);

char *excep(char *token, const char *substr);

void modify(char *line, int read);

char *file_mmap(const char *substr);

void printline(char *line, pat_info *p, int j, int exist, int blank, const char *path, FILE *fs);

void recursive(const char *basePath, const char *substr);

void print_filename(char *fname);

void search(char *path, const char *substr);

void find2(char *path, const char *substr);

void multiple_files(const char **path, const char *substr, int j);

void byte_offset(FILE *fp, char *line);

char *icase (const char* substr, char *tline, char *ptr);

void frecursive(const char *basePath, int num, char newpath[][100]);

int grep_exec(int argc, char *argv[]);

static ssize_t getline(char **lineptr, size_t *n, FILE *stream);

#endif /* LIBSYLIXOS_SYLIXOS_SHELL_COMMANDS_GREP_H_ */
