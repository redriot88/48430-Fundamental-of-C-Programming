#ifndef STORAGE_H
#define STORAGE_H

/* Storage helper for persisting the locker index and data.
 * This module uses the `index_t` defined in `locker.h` (the in-memory
 * linked-list index). It exposes two simple functions to save/load the
 * entire locker to a single binary file. The format is intentionally
 * straightforward for the assignment:
 *   [lockerHeader_t][entry1_meta][entry1_data]...[entryN_meta][entryN_data]
 */

#include "locker.h"

/* Save the entire locker to `path`. Returns 0 on success, non-zero on error.
 * This overwrites the target file. */
int storageSaveAll(const char *path, const index_t *idx, const char *masterPin);

/* Load the entire locker from `path` into an empty index. On success,
 * the function allocates nodes and data buffers; caller may use locker APIs
 * or lockerLoadIndex which wraps this. Returns 0 on success. */
int storageLoadAll(const char *path, index_t *idx, char *outMasterPin, size_t maxPinLen);

#endif /* STORAGE_H */
