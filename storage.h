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

/*
Save all locker data and index to disk
-will need path to output file
pointer to index structure containing locker entries
return 0 perhaps for success
*/

int storageSaveAll(const char *path, const index_t *idx);

/*
load data and index to disk
-path to input file
pointer to index structure to populate
return 0 perhaps for success
*/


int storageLoadAll(const char *path, index_t *idx);

#endif /* STORAGE_H */
