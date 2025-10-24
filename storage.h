#ifndef STORAGE_H
#define STORAGE_H

/*******************************************************************************
 * List header files 
*******************************************************************************/

#include "locker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * List preprocessing directives
*******************************************************************************/

#define MAX_FILENAME_LENGTH 64
#define MAX_FILES 50

/*******************************************************************************
 * List structs
*******************************************************************************/

/* Metadata on each stored file */

typedef struct {
  char filename[MAX_FILENAME_LENGTH]; /*original filename*/
  unsigned long size; /*compressed/encrypted data size*/
  unsigned long originalSize; /*size prior compression/encryption*/
  unsigned long offset; /*byte offset in data section*/
  unsigned int hash; /* for simple checksum/integrity check*/
} fileEntry_t;
  
/* Index structure (like a file allocation table)*/

typedef struct {
    unsigned int file_count;
    file_entry_t files[MAX_FILES];
} index_t;

/*******************************************************************************
 * Function Prototypes
*******************************************************************************/
int storageSaveAll(const char *path, const index_t *idx);
int storageLoadAll(const char *path, index_t *idx);

#endif /* STORAGE_H */
