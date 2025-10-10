#ifndef STORAGE_H
#define STORAGE_H
#include "locker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 64
#define MAX_FILES 50

typedef struct {
  char filename[MAX_FILE_LEN];

/* Placeholder function to persist full locker (data + index) */
int storageSaveAll(const char *path, const index_t *idx);
int storageLoadAll(const char *path, index_t *idx);

#endif /* STORAGE_H */
