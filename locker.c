/*
 * locker.c - Core implementation with persistence
 */

#include "locker.h"

/* Internal global index */
static index_t g_index = { NULL, 0, 0 };
static char g_masterPin[MAX_PIN] = {0};   /* loaded from file */
static FILE *g_lockerFile = NULL;

/* Accessor */
index_t *lockerGetIndex(void) {
    return &g_index;
}

/* Ensure dynamic capacity */
static int ensureCapacity(int needed) {
    if (g_index.capacity >= needed) return 0;
    int newCap = g_index.capacity ? g_index.capacity * 2 : 8;
    while (newCap < needed) newCap *= 2;
    indexEntry_t *tmp = realloc(g_index.entries, (size_t)newCap * sizeof(indexEntry_t));
    if (!tmp) return -1;
    g_index.entries = tmp;
    g_index.capacity = newCap;
    return 0;
}

/* File open helper */
static int openLockerFile(const char *lockerPath) {
    if (g_lockerFile) return 0;
    g_lockerFile = fopen(lockerPath, "r+b");
    if (!g_lockerFile) {
        g_lockerFile = fopen(lockerPath, "w+b");
        if (!g_lockerFile) return -1;
        lockerHeader_t hdr = { LOCKER_MAGIC, LOCKER_VERSION, 0u, "admin" };
        strncpy(g_masterPin, "admin", MAX_PIN-1);
        fwrite(&hdr, sizeof hdr, 1, g_lockerFile);
        fflush(g_lockerFile);
    }
    return 0;
}

/* Open locker */
int lockerOpen(const char *lockerPath, const char *pin) {
    if (!lockerPath || !*lockerPath) return -1;
    if (openLockerFile(lockerPath) != 0) return -1;
    if (lockerLoadIndex() != 0) return -1;
    if (pin && *pin) {
        if (strcmp(pin, g_masterPin) != 0) {
            DBG("PIN mismatch\n");
            return -1;
        }
    }
    return 0;
}

/* Close locker */
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

/* Change PIN */
int lockerChangePIN(const char *oldPin, const char *newPin) {
    if (strcmp(oldPin, g_masterPin) != 0) return -1;
    if (!newPin || !*newPin) return -2;
    strncpy(g_masterPin, newPin, MAX_PIN-1);
    g_masterPin[MAX_PIN-1] = '\0';
    return lockerSaveIndex(); /* persist immediately */
}

/* Add index entry helper */
static int addIndexEntry(const char *title, unsigned long origSize,
                         unsigned long storedSize, unsigned long offset,
                         unsigned int flags) {
    if (!title || !*title) return -1;
    if (ensureCapacity(g_index.count + 1) != 0) return -2;
    indexEntry_t *e = &g_index.entries[g_index.count];
    memset(e, 0, sizeof *e);
    strncpy(e->title, title, MAX_TITLE - 1);
    e->title[MAX_TITLE - 1] = '\0';
    e->originalSize = origSize;
    e->storedSize = storedSize;
    e->dataOffset = offset;
    e->flags = flags;
    g_index.count++;
    return 0;
}

/* Add file (stub for now) */
int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag) {
    if (!filepath || !*filepath || !title || !*title) return -1;
    unsigned int flags = 0u;
    if (compressFlag) flags |= FLAG_COMPRESSED;
    if (encryptFlag)  flags |= FLAG_ENCRYPTED;

    unsigned long origSize = 0ul;
    unsigned long storedSize = origSize;
    unsigned long offset = sizeof(lockerHeader_t) + (unsigned long)g_index.count * sizeof(indexEntry_t);

    int rc = addIndexEntry(title, origSize, storedSize, offset, flags);
    if (rc != 0) return rc;
    return lockerSaveIndex();
}

/* Extract file (stub) */
int lockerExtractFile(const char *title, const char *outputPath) {
    (void)title; (void)outputPath;
    DBG("lockerExtractFile stub called for %s\n", title ? title : "(null)");
    return 0;
}

/* Remove file */
int lockerRemoveFile(const char *title) {
    if (!title) return -1;
    for (int i=0;i<g_index.count;i++) {
        if (strcmp(g_index.entries[i].title, title)==0) {
            for (int j=i+1;j<g_index.count;j++) g_index.entries[j-1] = g_index.entries[j];
            g_index.count--;
            DBG("Removed entry %s\n", title);
            return lockerSaveIndex();
        }
    }
    return -2; /* not found */
}

/* List files */
void lockerList(void) {
    printf("\nStored Files (%d)\n", g_index.count);
    for (int i=0;i<g_index.count;i++) {
        indexEntry_t *e = &g_index.entries[i];
        printf("%2d. %-30s orig=%lu stored=%lu flags=0x%02X\n",
               i+1, e->title, e->originalSize, e->storedSize, e->flags);
    }
}

/* Search */
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

/* Save index + PIN */
int lockerSaveIndex(void) {
    if (!g_lockerFile) return -1;

    lockerHeader_t hdr;
    hdr.magic   = LOCKER_MAGIC;
    hdr.version = LOCKER_VERSION;
    hdr.count   = (unsigned int)g_index.count;
    strncpy(hdr.masterPin, g_masterPin, MAX_PIN - 1);
    hdr.masterPin[MAX_PIN - 1] = '\0';

    /* Write header */
    rewind(g_lockerFile);
    if (fwrite(&hdr, sizeof hdr, 1, g_lockerFile) != 1) return -2;

    /* Write index entries */
    size_t bytes = (size_t)g_index.count * sizeof(indexEntry_t);
    if (bytes > 0) {
        if (fwrite(g_index.entries, 1, bytes, g_lockerFile) != bytes) return -3;
    }

    fflush(g_lockerFile);
    DBG("lockerSaveIndex ok (count=%d, pin=%s)\n", g_index.count, g_masterPin);
    return 0;
}
/* Load header + index (also loads PIN) */
int lockerLoadIndex(void) {
    if (!g_lockerFile) return -1;

    lockerHeader_t hdr;
    rewind(g_lockerFile);
    if (fread(&hdr, sizeof hdr, 1, g_lockerFile) != 1) return -1;
    if (hdr.magic != LOCKER_MAGIC || hdr.version != LOCKER_VERSION) return -2;

    /* Restore PIN */
    strncpy(g_masterPin, hdr.masterPin, MAX_PIN - 1);
    g_masterPin[MAX_PIN - 1] = '\0';

    /* Ensure capacity and set count */
    if (ensureCapacity((int)hdr.count) != 0) return -3;
    g_index.count = (int)hdr.count;

    /* Read index entries */
    size_t bytes = (size_t)g_index.count * sizeof(indexEntry_t);
    if (bytes > 0) {
        if (fread(g_index.entries, 1, bytes, g_lockerFile) != bytes) return -4;
    }

    DBG("lockerLoadIndex ok (count=%d, pin=%s)\n", g_index.count, g_masterPin);
    return 0;
}

/* Print the interactive menu */
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