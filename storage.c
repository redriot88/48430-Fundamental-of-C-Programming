#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * This function prints the initial menu with all instructions on how to use
 * this program.
 * inputs:
 * - none
 * outputs:
 * - return values: 1 if error, 2 when writing total count, 3 to write all entires
*******************************************************************************/


/* Placeholder function to persist full locker (data + index) */
int storageSaveAll(const char *path, const index_t *idx){
  (void)path; (void)idx;
    /* TODO: implement real persistence */
    /* file pointer to open and save files */
    FILE *file = fopen(path, "wb");
    if (!FILE) {
        perror("Error opening file for writing");
        return 1;
        /* return 1 if error opening file */
    }
    /* write total count first */
    if (fwrite(&idx->count, sizeof(int), 1, file) !=1) {
        fclose(file);
        return 2;
        /* will need to see further explanations */
    }

    /* to write all entries */
    int i;
    for (i = 0; i < idx->count; i++){
        entry_t entry = idx->entires[i];

        /* encrypt data before saving perhaps */

        if (fwrite(&entry, sizeof(entry_t), 1, file) != 1){
            fclose(file);
            return 3;
        }
    }
    fclose(file);
    return 0;
}
/*******************************************************************************
 * This function loads all the files from the document safe locker
 * inputs: 
 * - none
 * outputs:
 * - return values: 1 if error, 2 when writing total count, 3 to write all entires
*******************************************************************************/

int storageLoadAll(const char *path, index_t *idx){
  (void)path; (void)idx;
    /* TODO: implement real load */
    /* file pointer to read and load files */
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("Error opening file for reading");
        return 1;
        /*to return 1 for error when opening file*/

        /*to return 2 for total count*/
        if (fread(&idx->count, sizeof(int), 1, file) != 1 {
            fclose(file);
            return 2;
        }

        /*to return 3 to read/load all entries*/
        int i;
        for (i = 0; i < idx->count; i++){
            if fread($idx->entiries[i], sizeof(entry_t), 1, file) !=) {
                fclose(file);
                return 3;
            }

        /* can have option to decrypt after reading
        xorEncryptDecrypt((unsigned char *)idx->entires[i].data,strlen(idx->entries[i].data), "masterPIN"); */
        }
    fclose(file);
    return 0;
}
