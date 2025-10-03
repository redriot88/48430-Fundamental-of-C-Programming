#ifndef STORAGE_H
#define STORAGE_H
#include "locker.h"

/* Placeholder function to persist full locker (data + index) */
int storage_save_all(const char *path, const index_t *idx);
int storage_load_all(const char *path, index_t *idx);

#endif /* STORAGE_H */
