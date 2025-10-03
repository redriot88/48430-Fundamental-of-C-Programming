/*
 * locker.c - Core implementation (stubs for Checkpoint 1)
 */

#include "locker.h"

/* Internal global index */
static index_t g_index = { NULL, 0, 0 };
static char g_masterPin[MAX_PIN] = "admin"; /* placeholder; later hash & persist */
static FILE *g_lockerFile = NULL;            /* placeholder for actual storage file */

index_t *lockerGetIndex(void) {
    return &g_index;
}

static int ensureCapacity(int needed) {
    if (g_index.capacity >= needed) return 0;
    int newCap = g_index.capacity ? g_index.capacity * 2 : 8;
    while (newCap < needed) newCap *= 2;
    indexEntry_t *tmp = (indexEntry_t*)realloc(g_index.entries, (size_t)newCap * sizeof(indexEntry_t));
    if (!tmp) return -1;
    g_index.entries = tmp;
    g_index.capacity = newCap;
    return 0;
}

int lockerOpen(const char *lockerPath, const char *pin) {
    (void)lockerPath; /* TODO: open/create file, read header */
    if (pin && *pin) {
        /* simple PIN check for now */
        if (strcmp(pin, g_masterPin) != 0) {
            DBG("PIN mismatch\n");
            return -1;
        }
    }
    /* Load index stub */
    return lockerLoadIndex();
}

int lockerClose(void) {
    lockerSaveIndex();
    if (g_lockerFile) {
        fclose(g_lockerFile);
        g_lockerFile = NULL;
    }
    free(g_index.entries);
    g_index.entries = NULL;
    g_index.count = g_index.capacity = 0;
    return 0;
}

int lockerChangePIN(const char *oldPin, const char *newPin) {
    if (strcmp(oldPin, g_masterPin) != 0) return -1;
    if (!newPin || !*newPin) return -2;
    strncpy(g_masterPin, newPin, MAX_PIN-1);
    g_masterPin[MAX_PIN-1] = '\0';
    /* TODO: re-encrypt all stored files with new PIN */
    return 0;
}

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag) {
    (void)filepath; (void)title; (void)compressFlag; (void)encryptFlag;
    /* TODO: read file, compress, encrypt, append to storage, update index */
    DBG("lockerAddFile stub called for %s\n", title ? title : "(null)");
    return 0;
}

int lockerExtractFile(const char *title, const char *outputPath) {
    (void)title; (void)outputPath;
    /* TODO: locate entry, read bytes, decrypt, decompress, write out */
    DBG("lockerExtractFile stub called for %s\n", title ? title : "(null)");
    return 0;
}

int lockerRemoveFile(const char *title) {
    if (!title) return -1;
    for (int i=0;i<g_index.count;i++) {
        if (strcmp(g_index.entries[i].title, title)==0) {
            /* shift */
            for (int j=i+1;j<g_index.count;j++) g_index.entries[j-1] = g_index.entries[j];
            g_index.count--;
            DBG("Removed entry %s\n", title);
            return 0;
        }
    }
    return -2; /* not found */
}

void lockerList(void) {
    printf("\nStored Files (%d)\n", g_index.count);
    for (int i=0;i<g_index.count;i++) {
        indexEntry_t *e = &g_index.entries[i];
        printf("%2d. %-30s orig=%lu stored=%lu flags=0x%02X\n", i+1, e->title, e->originalSize, e->storedSize, e->flags);
    }
}

int lockerSearch(const char *pattern) {
    if (!pattern || !*pattern) return 0;
    int matches = 0;
    for (int i=0;i<g_index.count;i++) {
        if (strstr(g_index.entries[i].title, pattern)) {
            printf("Match: %s\n", g_index.entries[i].title);
            matches++;
        }
    }
    return matches;
}

int lockerSaveIndex(void) {
    /* TODO: persist index to file */
    DBG("lockerSaveIndex stub (entries=%d)\n", g_index.count);
    return 0;
}

int lockerLoadIndex(void) {
    /* TODO: load index from file; for now empty */
    DBG("lockerLoadIndex stub (starting empty)\n");
    return 0;
}

void printMenu(void) {
    printf("\nPersonal Document Locker\n"
           "1. Add file\n"
           "2. Extract file\n"
           "3. Remove file\n"
           "4. List files\n"
           "5. Search by filename\n"
           "6. Change master PIN\n"
           "7. Quit\n");
    printf("Select option: ");
}

