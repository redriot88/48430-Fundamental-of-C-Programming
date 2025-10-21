#include "storage.h"
#include <stdio.h>

int storageSaveAll(const char *path, const index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real persistence */
    return 0;
}

int storageLoadAll(const char *path, index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real load */
    return 0;
}
