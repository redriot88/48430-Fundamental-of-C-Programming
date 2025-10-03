#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long util_timestamp(void); /* placeholder simple counter */
int util_readFile(const char *path, unsigned char **buffer, size_t *size);
int util_writeFile(const char *path, const unsigned char *buffer, size_t size);

#endif /* UTIL_H */
