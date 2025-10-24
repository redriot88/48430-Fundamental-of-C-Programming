/*
 * locker.c - Core implementation (stubs for Checkpoint 1)
 */

#include "locker.h"
#include "compress.h"
#include "crypto.h"
#include "util.h"

/* Internal global index */
static index_t g_index = { NULL, 0, 0 };
static char g_masterPin[MAX_PIN] = "admin"; /* placeholder; later hash & persist */
static FILE *g_lockerFile = NULL;            /* placeholder for actual storage file */
static int g_role = ROLE_PUBLIC;             /* current session role */

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
            DBG1((stderr, "[DBG] %s", "PIN mismatch\n"));
            return -1;
        }
        g_role = ROLE_ADMIN;
    } else {
        g_role = ROLE_PUBLIC;
    }
    /* Load index stub */
    return lockerLoadIndex();
}

int lockerGetRole(void) {
    return g_role;
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

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic) {
    unsigned char *inBuf;
    size_t inSize;
    unsigned char *workBuf;
    size_t workCap;
    size_t workSize;
    unsigned char key[64];
    indexEntry_t *e;
    int rc;
    size_t needed;
    if (g_role != ROLE_ADMIN) return -3; /* only admin can add */
    if (!filepath || !title || !*title) return -1;
    rc = util_readFile(filepath, &inBuf, &inSize);
    if (rc != 0) return rc;
    /* allocate worst-case for RLE: ~2*n */
    needed = inSize * 2u + 2u;
    workCap = needed;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) { free(inBuf); return -5; }
    /* Optional compression */
    if (compressFlag) {
        workSize = rle_compress(inBuf, inSize, workBuf, workCap);
        if (workSize == 0) { /* fallback: store raw if compression failed */
            memcpy(workBuf, inBuf, inSize);
            workSize = inSize;
            compressFlag = 0;
        }
    } else {
        memcpy(workBuf, inBuf, inSize);
        workSize = inSize;
    }
    /* Optional encryption */
    if (encryptFlag) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(inBuf); free(workBuf); return -6; }
        xor_cipher(workBuf, workSize, key, sizeof key);
    }
    /* grow index */
    if (g_index.count + 1 > g_index.capacity) {
        int newCap = g_index.capacity ? g_index.capacity * 2 : 8;
        indexEntry_t *tmp = (indexEntry_t*)realloc(g_index.entries, (size_t)newCap * sizeof(indexEntry_t));
        if (!tmp) { free(inBuf); free(workBuf); return -7; }
        g_index.entries = tmp;
        g_index.capacity = newCap;
    }
    e = &g_index.entries[g_index.count++];
    memset(e, 0, sizeof(*e));
    strncpy(e->title, title, MAX_TITLE-1);
    e->originalSize = (unsigned long)inSize;
    e->storedSize = (unsigned long)workSize;
    e->flags = (compressFlag?FLAG_COMPRESSED:0u) | (encryptFlag?FLAG_ENCRYPTED:0u);
    e->isPublic = makePublic ? 1 : 0;
    e->data = (unsigned char*)malloc(workSize);
    if (!e->data) { g_index.count--; free(inBuf); free(workBuf); return -8; }
    memcpy(e->data, workBuf, workSize);
    free(inBuf);
    free(workBuf);
    DBG1((stderr, "[DBG] Added entry %s (orig=%lu stored=%lu flags=0x%X public=%d)\n", e->title, e->originalSize, e->storedSize, e->flags, e->isPublic));
    return 0;
}

int lockerExtractFile(const char *title, const char *outputPath) {
    int i;
    unsigned char *buf;
    size_t n;
    unsigned char key[64];
    unsigned char *tmp;
    size_t outN;
    if (!title || !outputPath) return -1;
    for (i=0;i<g_index.count;i++) {
        indexEntry_t *e = &g_index.entries[i];
        if (strcmp(e->title, title)==0) {
            if (g_role == ROLE_PUBLIC && !e->isPublic) return -3; /* not allowed */
            n = (size_t)e->storedSize;
            buf = (unsigned char*)malloc(n);
            if (!buf) return -4;
            memcpy(buf, e->data, n);
            /* decrypt if needed */
            if (e->flags & FLAG_ENCRYPTED) {
                if (derive_key(g_masterPin, key, sizeof key)==0) { free(buf); return -5; }
                xor_cipher(buf, n, key, sizeof key);
            }
            /* decompress if needed */
            if (e->flags & FLAG_COMPRESSED) {
                tmp = (unsigned char*)malloc((size_t)e->originalSize);
                if (!tmp) { free(buf); return -6; }
                outN = rle_decompress(buf, n, tmp, (size_t)e->originalSize);
                free(buf);
                if (outN != (size_t)e->originalSize) { free(tmp); return -7; }
                buf = tmp; n = outN;
            }
            if (util_writeFile(outputPath, buf, n) != 0) { free(buf); return -8; }
            free(buf);
            DBG1((stderr, "[DBG] Extracted %s to %s\n", title, outputPath));
            return 0;
        }
    }
    return -2;
}

int lockerRemoveFile(const char *title) {
    int i;
    if (!title) return -1;
    if (g_role != ROLE_ADMIN) return -3;
    for (i=0;i<g_index.count;i++) {
        if (strcmp(g_index.entries[i].title, title)==0) {
            /* shift */
            int j;
            if (g_index.entries[i].data) { free(g_index.entries[i].data); }
            for (j=i+1;j<g_index.count;j++) g_index.entries[j-1] = g_index.entries[j];
            g_index.count--;
            DBG1((stderr, "[DBG] Removed entry %s\n", title));
            return 0;
        }
    }
    return -2; /* not found */
}

void lockerList(void) {
    int i;
    int shown = 0;
    printf("\nStored Files (%d)\n", g_index.count);
    for (i=0;i<g_index.count;i++) {
        indexEntry_t *e = &g_index.entries[i];
        if (g_role == ROLE_PUBLIC && !e->isPublic) continue;
        printf("%2d. %-30s orig=%lu stored=%lu flags=0x%02X vis=%s\n", i+1, e->title, e->originalSize, e->storedSize, e->flags, e->isPublic?"public":"private");
        shown++;
    }
    if (g_role == ROLE_PUBLIC && shown==0) printf("(no public files)\n");
}

int lockerSearch(const char *pattern) {
    int matches = 0;
    int i;
    if (!pattern || !*pattern) return 0;
    for (i=0;i<g_index.count;i++) {
        if (g_role == ROLE_PUBLIC && !g_index.entries[i].isPublic) continue;
        if (strstr(g_index.entries[i].title, pattern)) {
            printf("Match: %s\n", g_index.entries[i].title);
            matches++;
        }
    }
    return matches;
}

int lockerSaveIndex(void) {
    /* TODO: persist index to file */
    DBG1((stderr, "[DBG] lockerSaveIndex stub (entries=%d)\n", g_index.count));
    return 0;
}

int lockerLoadIndex(void) {
    /* TODO: load index from file; for now empty */
    DBG1((stderr, "[DBG] lockerLoadIndex stub (starting empty)\n"));
    return 0;
}

void printMenu(void) {
    printf("\nPersonal Document Locker (%s)\n",
        (g_role==ROLE_ADMIN?"admin":"public"));
    printf("1. Add file %s\n",
        (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("2. Extract file\n");
    printf("3. Remove file %s\n",
        (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("4. List files\n");
    printf("5. Search by filename\n");
    printf("6. Change master PIN %s\n",
        (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("7. Logout\n");
    printf("8. Quit\n");
    printf("Select option: ");
}

