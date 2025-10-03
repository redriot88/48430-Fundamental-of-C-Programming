#include "storage.h"
#include <stdio.h>

int storage_save_all(const char *path, const index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real persistence */
    return 0;
}

int storage_load_all(const char *path, index_t *idx) {
    (void)path; (void)idx;
    /* TODO: implement real load */
    return 0;
}
