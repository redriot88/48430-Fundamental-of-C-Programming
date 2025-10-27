/*
 * locker.c - Core implementation using a linked-list index
 */

#include "locker.h"
#include "compress.h"
#include "crypto.h"
#include "util.h"
#include "storage.h"

/* Internal global index */
static index_t g_index = { NULL, 0 };
static char g_masterPin[MAX_PIN] = "admin"; /* placeholder; later hash & persist */
static FILE *g_lockerFile = NULL;            /* optional backing file */
static char g_lockerPath[1024] = {0};        /* path to current locker file */
static int g_role = ROLE_PUBLIC;             /* current session role */

/* Accessor */
index_t *lockerGetIndex(void) { return &g_index; }
int lockerGetRole(void) { return g_role; }

/* File open helper (creates file if missing) */
static int openLockerFile(const char *lockerPath) {
    lockerHeader_t hdr;
    if (g_lockerFile) return 0;
    g_lockerFile = fopen(lockerPath, "r+b");
    if (!g_lockerFile) {
        g_lockerFile = fopen(lockerPath, "w+b");
        if (!g_lockerFile) return -1;
        hdr.magic = LOCKER_MAGIC; hdr.version = LOCKER_VERSION; hdr.count = 0; 
        strncpy(hdr.masterPin, "admin", MAX_PIN-1); hdr.masterPin[MAX_PIN-1]='\0';
        strncpy(g_masterPin, "admin", MAX_PIN-1); g_masterPin[MAX_PIN-1]='\0';
        fwrite(&hdr, sizeof hdr, 1, g_lockerFile); fflush(g_lockerFile);
    }
    /* remember path for persistence helpers */
    if (lockerPath && lockerPath[0]) strncpy(g_lockerPath, lockerPath, sizeof(g_lockerPath)-1);
    return 0;
}

int lockerOpen(const char *lockerPath, const char *pin) {
    if (!lockerPath || !*lockerPath) return -1;
    if (openLockerFile(lockerPath) != 0) return -1;
    /* attempt to load persisted index; non-fatal if it fails */
    if (lockerLoadIndex() != 0) {
        DBG("[DBG] lockerLoadIndex: no persisted data or error\n");
    }
    if (pin && *pin) {
        if (strcmp(pin, g_masterPin) != 0) { DBG("[DBG] PIN mismatch\n"); return -1; }
        g_role = ROLE_ADMIN;
    } else {
        g_role = ROLE_PUBLIC;
    }
    return 0;
}

int lockerClose(void) {
    lockerSaveIndex();
    if (g_lockerFile) { fclose(g_lockerFile); g_lockerFile = NULL; }
    /* free list */
    indexNode_t *n;
    n = g_index.head;
    while (n) {
        indexNode_t *nx = n->next;
        if (n->entry.data) free(n->entry.data);
        free(n);
        n = nx;
    }
    g_index.head = NULL; g_index.count = 0;
    return 0;
}

int lockerChangePIN(const char *oldPin, const char *newPin) {
    if (!oldPin || !newPin) return -1;
    if (strcmp(oldPin, g_masterPin) != 0) return -2;
    strncpy(g_masterPin, newPin, MAX_PIN-1); g_masterPin[MAX_PIN-1] = '\0';
    return lockerSaveIndex();
}

/* Find node by title: returns node and previous via outPrev (may be NULL) */
static indexNode_t *findNode(const char *title, indexNode_t **outPrev) {
    indexNode_t *p = NULL; indexNode_t *n = g_index.head;
    while (n) {
        if (strcmp(n->entry.title, title) == 0) { if (outPrev) *outPrev = p; return n; }
        p = n; n = n->next;
    }
    if (outPrev) *outPrev = NULL; return NULL;
}

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic) {
    unsigned char *inBuf = NULL;
    size_t inSize = 0;
    unsigned char *workBuf = NULL;
    size_t workCap, workSize;
    unsigned char key[128];
    int rc;
    indexNode_t *node;

    if (g_role != ROLE_ADMIN) return -3; /* only admin */
    if (!filepath || !title || !*title) return -1;
    rc = util_readFile(filepath, &inBuf, &inSize);
    if (rc != 0) return rc;
    workCap = inSize * 2 + 4;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) { free(inBuf); return -5; }
    if (compressFlag) {
        workSize = rle_compress(inBuf, inSize, workBuf, workCap);
        if (workSize == 0) { memcpy(workBuf, inBuf, inSize); workSize = inSize; compressFlag = 0; }
    } else { memcpy(workBuf, inBuf, inSize); workSize = inSize; }
    if (encryptFlag) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(inBuf); free(workBuf); return -6; }
        xor_cipher(workBuf, workSize, key, sizeof key);
    }
    /* create node */
    node = (indexNode_t*)malloc(sizeof(indexNode_t));
    if (!node) { free(inBuf); free(workBuf); return -7; }
    memset(&node->entry, 0, sizeof(node->entry));
    strncpy(node->entry.title, title, MAX_TITLE-1);
    node->entry.originalSize = (unsigned long)inSize;
    node->entry.storedSize = (unsigned long)workSize;
    node->entry.flags = (compressFlag?FLAG_COMPRESSED:0u) | (encryptFlag?FLAG_ENCRYPTED:0u);
    node->entry.isPublic = makePublic ? 1 : 0;
    node->entry.data = (unsigned char*)malloc(workSize);
    if (!node->entry.data) { free(node); free(inBuf); free(workBuf); return -8; }
    memcpy(node->entry.data, workBuf, workSize);
    node->next = g_index.head; g_index.head = node; g_index.count++;
    free(inBuf); free(workBuf);
    DBG("[DBG] Added entry %s (orig=%lu stored=%lu flags=0x%X public=%d)\n", node->entry.title, node->entry.originalSize, node->entry.storedSize, node->entry.flags, node->entry.isPublic);
    return 0;
}

int lockerExtractFile(const char *title, const char *outputPath) {
    indexNode_t *n;
    unsigned char *buf = NULL, *tmp = NULL;
    size_t nbytes, outN;
    unsigned char key[128];

    if (!title || !outputPath) return -1;
    n = findNode(title, NULL);
    if (!n) return -2;
    if (g_role == ROLE_PUBLIC && !n->entry.isPublic) return -3;
    DBG("[DBG] lockerExtractFile: found entry '%s' stored=%lu orig=%lu flags=0x%X public=%d\n", n->entry.title, n->entry.storedSize, n->entry.originalSize, n->entry.flags, n->entry.isPublic);
    nbytes = (size_t)n->entry.storedSize;
    buf = (unsigned char*)malloc(nbytes);
    if (!buf) return -4;
    memcpy(buf, n->entry.data, nbytes);
    if (n->entry.flags & FLAG_ENCRYPTED) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(buf); return -5; }
        xor_cipher(buf, nbytes, key, sizeof key);
    }
    if (n->entry.flags & FLAG_COMPRESSED) {
        tmp = (unsigned char*)malloc((size_t)n->entry.originalSize);
        if (!tmp) { free(buf); return -6; }
        outN = rle_decompress(buf, nbytes, tmp, (size_t)n->entry.originalSize);
        free(buf);
        if (outN != (size_t)n->entry.originalSize) { free(tmp); return -7; }
        buf = tmp; nbytes = outN;
    }
    if (util_writeFile(outputPath, buf, nbytes) != 0) { free(buf); return -8; }
    free(buf);
    DBG("[DBG] Extracted %s to %s\n", title, outputPath);
    return 0;
}

int lockerRemoveFile(const char *title) {
    indexNode_t *prev = NULL; indexNode_t *n;
    if (!title) return -1;
    if (g_role != ROLE_ADMIN) return -3;
    n = findNode(title, &prev);
    if (!n) return -2;
    if (prev) prev->next = n->next; else g_index.head = n->next;
    if (n->entry.data) free(n->entry.data);
    free(n);
    g_index.count--;
    DBG("[DBG] Removed entry %s\n", title);
    return 0;
}

void lockerList(void) {
    indexNode_t *n = g_index.head;
    int idx = 1; int shown = 0;
    printf("\nStored Files (%d)\n", g_index.count);
    while (n) {
        if (g_role == ROLE_PUBLIC && !n->entry.isPublic) { n = n->next; idx++; continue; }
        printf("%2d. %-30s orig=%lu stored=%lu flags=0x%02X vis=%s\n", idx, n->entry.title, n->entry.originalSize, n->entry.storedSize, n->entry.flags, n->entry.isPublic?"public":"private");
        shown++; n = n->next; idx++;
    }
    if (g_role == ROLE_PUBLIC && shown==0) printf("(no public files)\n");
}

int lockerSearch(const char *pattern) {
    int matches = 0; indexNode_t *n = g_index.head;
    if (!pattern || !*pattern) return 0;
    while (n) {
        if (g_role == ROLE_PUBLIC && !n->entry.isPublic) { n = n->next; continue; }
        if (strstr(n->entry.title, pattern)) { printf("Match: %s\n", n->entry.title); matches++; }
        n = n->next;
    }
    return matches;
}

int lockerSaveIndex(void) {
    if (g_lockerPath[0] == '\0') { DBG("[DBG] no locker path set\n"); return -1; }
    DBG("[DBG] saving index to %s (entries=%d)\n", g_lockerPath, g_index.count);
    return storageSaveAll(g_lockerPath, &g_index, g_masterPin);
}

int lockerLoadIndex(void) {
    if (g_lockerPath[0] == '\0') { DBG("[DBG] no locker path set\n"); return -1; }
    DBG("[DBG] loading index from %s\n", g_lockerPath);
    return storageLoadAll(g_lockerPath, &g_index, g_masterPin, sizeof(g_masterPin));
}

void printMenu(void) {
    printf("\nPersonal Document Locker (%s)\n", (g_role==ROLE_ADMIN?"admin":"public"));
    printf("1. Add file %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("2. Extract file\n");
    printf("3. Remove file %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("4. List files\n");
    printf("5. Search by filename\n");
    printf("6. Change master PIN %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("7. Logout\n");
    printf("8. Quit\n");
    printf("Select option: ");
}