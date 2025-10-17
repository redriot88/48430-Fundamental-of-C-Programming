#include "storage.h"
#include <stdio.h>

int storageSaveAll(const char *path, const index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real persistence */
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
/*
load locker data and index from disk
returns 0 for successs, otherwise failure
*/

int storageLoadAll(const char *path, index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real load */
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("Error opening file for reading");
        return 1;
        /*to return 1 for error when opening file*/
        if (fread(&idx->count, sizeof(int), 1, file) != 1 {
            fclose(file);
            return 2;
        }

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
