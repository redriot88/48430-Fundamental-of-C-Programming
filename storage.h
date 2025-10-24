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

int  storage_loadIndex(const char *index_path, index_t *idx);
int  storage_saveIndex(const char *index_path, const index_t *idx);
int  storage_addFile(index_t *idx, const char *filename, unsigned long size,
                     unsigned long original_size, unsigned long offset, unsigned int hash);
int  storage_removeFile(index_t *idx, const char *filename);
const file_entry_t* storage_findFile(const index_t *idx, const char *filename);
void storage_listFiles(const index_t *idx);
unsigned int storage_simpleHash(const void *data, size_t len);

#endif /* STORAGE_H */
