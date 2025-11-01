/*
 * util.h
 * Small helper functions for file I/O and simple utilities used by the
 * locker program. These are intentionally minimal to keep focus on the
 * assignment tasks (indexing, compression, encryption).
 */

#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Runtime debug flag (0=off, 1=on). Controlled by CLI (--debug or 'debug'). */
extern int g_runtimeDebug;

unsigned long util_timestamp(void); /* placeholder simple counter */
int util_readFile(const char *path, unsigned char **buffer, size_t *size);
int util_writeFile(const char *path, const unsigned char *buffer, size_t size);

/* Best-effort creation of 'storage' directory (POSIX). Returns 1 always. */
int util_ensureStorageDir(void);

#endif /* UTIL_H */
