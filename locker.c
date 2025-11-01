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
    indexNode_t *n;
    lockerSaveIndex();
    if (g_lockerFile) { fclose(g_lockerFile); g_lockerFile = NULL; }
    /* free list */
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
    unsigned char oldKey[128];
    unsigned char newKey[128];
    size_t klen;
    indexNode_t *n;
    if (!oldPin || !newPin) return -1;
    if (strcmp(oldPin, g_masterPin) != 0) return -2;
    klen = sizeof oldKey;
    if (derive_key(oldPin, oldKey, klen) == 0) return -3;
    if (derive_key(newPin, newKey, klen) == 0) return -4;
    /* Re-encrypt all encrypted entries from old key to new key, in-place */
    n = g_index.head;
    while (n) {
        if ((n->entry.flags & FLAG_ENCRYPTED) && n->entry.storedSize > 0) {
            xor_cipher(n->entry.data, (size_t)n->entry.storedSize, oldKey, klen);
            xor_cipher(n->entry.data, (size_t)n->entry.storedSize, newKey, klen);
        }
        n = n->next;
    }
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
    if (outPrev) {
        *outPrev = NULL;
    }
    return NULL;
}

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic) {
    unsigned char *inBuf = NULL;
    size_t inSize = 0;
    unsigned char *workBuf = NULL;
    size_t workCap, workSize;
    unsigned char key[128];
    unsigned int hash32 = 0u;
    int rc;
    indexNode_t *node;

    if (g_role != ROLE_ADMIN) return -3; /* only admin */
    if (!title || !*title) return -1;
    /* Allow empty filepath to create an empty file entry */
    if (!filepath || !*filepath) {
        inBuf = (unsigned char*)""; /* safe for memcpy with size 0 */
        inSize = 0;
        rc = 0;
    } else {
        rc = util_readFile(filepath, &inBuf, &inSize);
        if (rc != 0) return rc;
    }
    if (inSize > 0) {
        hash32 = (unsigned int)compute_file_hash(inBuf, inSize);
    } else {
        hash32 = 0u;
    }
    workCap = inSize * 2 + 4;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) { free(inBuf); return -5; }
    if (compressFlag && inSize > 0) {
        workSize = rle_compress(inBuf, inSize, workBuf, workCap);
        if (workSize == 0) { memcpy(workBuf, inBuf, inSize); workSize = inSize; compressFlag = 0; }
    } else { memcpy(workBuf, inBuf, inSize); workSize = inSize; }
    if (encryptFlag && workSize > 0) {
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
    node->entry.hash = hash32;
    node->entry.isPublic = makePublic ? 1 : 0;
    if (workSize > 0) {
        node->entry.data = (unsigned char*)malloc(workSize);
        if (!node->entry.data) { free(node); free(inBuf); free(workBuf); return -8; }
        memcpy(node->entry.data, workBuf, workSize);
    } else {
        node->entry.data = NULL; /* zero-length payload */
    }
    node->next = g_index.head; g_index.head = node; g_index.count++;
    /* Only free inBuf if it was allocated by util_readFile. When filepath is empty,
       inBuf points to a string literal and must not be freed. */
    if (filepath && *filepath) free(inBuf);
    free(workBuf);
    DBG("[DBG] Added entry %s (orig=%lu stored=%lu flags=0x%X public=%d)\n", node->entry.title, node->entry.originalSize, node->entry.storedSize, node->entry.flags, node->entry.isPublic);
    return 0;
}

int lockerExtractFile(const char *title, const char *outputPath) {
    indexNode_t *n;
    unsigned char *buf = NULL, *tmp = NULL;
    size_t nbytes, outN;
    unsigned char key[128];
    unsigned int calcHash;

    if (!title || !outputPath) return -1;
    n = findNode(title, NULL);
    if (!n) return -2;
    if (g_role == ROLE_PUBLIC && !n->entry.isPublic) return -3;
    DBG("[DBG] lockerExtractFile: found entry '%s' stored=%lu orig=%lu flags=0x%X public=%d\n", n->entry.title, n->entry.storedSize, n->entry.originalSize, n->entry.flags, n->entry.isPublic);
    nbytes = (size_t)n->entry.storedSize;
    if (nbytes > 0) {
        buf = (unsigned char*)malloc(nbytes);
        if (!buf) return -4;
        memcpy(buf, n->entry.data, nbytes);
    } else {
        buf = NULL; /* zero-length content */
    }
    if ((n->entry.flags & FLAG_ENCRYPTED) && nbytes > 0) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(buf); return -5; }
        xor_cipher(buf, nbytes, key, sizeof key);
    }
    if ((n->entry.flags & FLAG_COMPRESSED) && nbytes > 0) {
        tmp = (unsigned char*)malloc((size_t)n->entry.originalSize);
        if (!tmp) { free(buf); return -6; }
        outN = rle_decompress(buf, nbytes, tmp, (size_t)n->entry.originalSize);
        free(buf);
        if (outN != (size_t)n->entry.originalSize) { free(tmp); return -7; }
        buf = tmp; nbytes = outN;
    }
    /* Integrity check on the original content */
    if (n->entry.originalSize > 0) {
        calcHash = (unsigned int)compute_file_hash(buf, (size_t)n->entry.originalSize);
        if (calcHash != n->entry.hash) { if (buf) free(buf); return -9; }
    }
    if (util_writeFile(outputPath, buf, nbytes) != 0) { if (buf) free(buf); return -8; }
    if (buf) free(buf);
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
        printf("%2d. %-30s orig=%lu stored=%lu flags=0x%02X hash=0x%08X vis=%s\n", idx, n->entry.title, n->entry.originalSize, n->entry.storedSize, n->entry.flags, n->entry.hash, n->entry.isPublic?"public":"private");
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
    printf("1. Add file (type content) %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("2. View file (decrypt+decompress)\n");
    printf("3. Remove file %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("4. List files\n");
    printf("5. Search by filename\n");
    printf("6. Change master PIN %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("7. Edit file %s\n", (g_role==ROLE_ADMIN?"":"(admin only)"));
    printf("8. Logout\n");
    printf("9. Quit\n");
    printf("Select option: ");
}

int lockerEditFile(const char *title, const char *newTitle, const char *filepath, int compressFlag, int encryptFlag, int makePublic) {
    indexNode_t *n;
    unsigned char *inBuf;
    size_t inSize;
    unsigned char *workBuf;
    size_t workCap, workSize;
    unsigned char key[128];
    unsigned int hash32;
    int rc;

    if (g_role != ROLE_ADMIN) return -3;
    if (!title || !*title) return -1;
    n = findNode(title, NULL);
    if (!n) return -2;

    /* Load new content */
    inBuf = NULL; inSize = 0; workBuf = NULL; workCap = 0; workSize = 0; hash32 = 0u;
    if (!filepath || !*filepath) {
        inBuf = (unsigned char*)""; inSize = 0; rc = 0;
    } else {
        rc = util_readFile(filepath, &inBuf, &inSize);
        if (rc != 0) return rc;
    }
    if (inSize > 0) hash32 = (unsigned int)compute_file_hash(inBuf, inSize);

    workCap = inSize * 2 + 4;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) { if (filepath && *filepath) free(inBuf); return -5; }
    if (compressFlag && inSize > 0) {
        workSize = rle_compress(inBuf, inSize, workBuf, workCap);
        if (workSize == 0) { memcpy(workBuf, inBuf, inSize); workSize = inSize; compressFlag = 0; }
    } else { memcpy(workBuf, inBuf, inSize); workSize = inSize; }
    if (encryptFlag && workSize > 0) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { if (filepath && *filepath) free(inBuf); free(workBuf); return -6; }
        xor_cipher(workBuf, workSize, key, sizeof key);
    }

    /* Replace entry data */
    if (n->entry.data) free(n->entry.data);
    n->entry.data = NULL;
    if (workSize > 0) {
        n->entry.data = (unsigned char*)malloc(workSize);
        if (!n->entry.data) { if (filepath && *filepath) free(inBuf); free(workBuf); return -8; }
        memcpy(n->entry.data, workBuf, workSize);
    }
    if (newTitle && *newTitle) { strncpy(n->entry.title, newTitle, MAX_TITLE-1); n->entry.title[MAX_TITLE-1] = '\0'; }
    n->entry.originalSize = (unsigned long)inSize;
    n->entry.storedSize = (unsigned long)workSize;
    n->entry.flags = (compressFlag?FLAG_COMPRESSED:0u) | (encryptFlag?FLAG_ENCRYPTED:0u);
    n->entry.hash = hash32;
    n->entry.isPublic = makePublic ? 1 : 0;

    if (filepath && *filepath) free(inBuf);
    free(workBuf);
    DBG("[DBG] Edited entry %s (newTitle=%s)\n", title, (newTitle&&*newTitle)?newTitle:title);
    return 0;
}

int lockerAddContent(const char *title, const unsigned char *buf, unsigned long size, int compressFlag, int encryptFlag, int makePublic) {
    unsigned char *workBuf;
    size_t workCap, workSize;
    unsigned char key[128];
    unsigned int hash32;
    indexNode_t *node;

    if (g_role != ROLE_ADMIN) return -3;
    if (!title || !*title || (!buf && size>0)) return -1;
    hash32 = (size>0)?(unsigned int)compute_file_hash(buf, (size_t)size):0u;
    workCap = (size_t)size * 2u + 4u;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) return -5;
    if (compressFlag && size > 0u) {
        workSize = rle_compress(buf, (size_t)size, workBuf, workCap);
        if (workSize == 0) { memcpy(workBuf, buf, (size_t)size); workSize = (size_t)size; compressFlag = 0; }
    } else { memcpy(workBuf, buf, (size_t)size); workSize = (size_t)size; }
    if (encryptFlag && workSize > 0u) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(workBuf); return -6; }
        xor_cipher(workBuf, workSize, key, sizeof key);
    }
    node = (indexNode_t*)malloc(sizeof(indexNode_t));
    if (!node) { free(workBuf); return -7; }
    memset(&node->entry, 0, sizeof(node->entry));
    strncpy(node->entry.title, title, MAX_TITLE-1);
    node->entry.originalSize = (unsigned long)size;
    node->entry.storedSize = (unsigned long)workSize;
    node->entry.flags = (compressFlag?FLAG_COMPRESSED:0u) | (encryptFlag?FLAG_ENCRYPTED:0u);
    node->entry.hash = hash32;
    node->entry.isPublic = makePublic ? 1 : 0;
    if (workSize > 0u) {
        node->entry.data = (unsigned char*)malloc(workSize);
        if (!node->entry.data) { free(node); free(workBuf); return -8; }
        memcpy(node->entry.data, workBuf, workSize);
    } else {
        node->entry.data = NULL;
    }
    node->next = g_index.head; g_index.head = node; g_index.count++;
    free(workBuf);
    return 0;
}

int lockerEditContent(const char *title, const char *newTitle, const unsigned char *buf, unsigned long size, int compressFlag, int encryptFlag, int makePublic) {
    indexNode_t *n;
    unsigned char *workBuf;
    size_t workCap, workSize;
    unsigned char key[128];
    unsigned int hash32;

    if (g_role != ROLE_ADMIN) return -3;
    if (!title || !*title || (!buf && size>0)) return -1;
    n = findNode(title, NULL);
    if (!n) return -2;
    hash32 = (size>0)?(unsigned int)compute_file_hash(buf, (size_t)size):0u;
    workCap = (size_t)size * 2u + 4u;
    workBuf = (unsigned char*)malloc(workCap);
    if (!workBuf) return -5;
    if (compressFlag && size > 0u) {
        workSize = rle_compress(buf, (size_t)size, workBuf, workCap);
        if (workSize == 0) { memcpy(workBuf, buf, (size_t)size); workSize = (size_t)size; compressFlag = 0; }
    } else { memcpy(workBuf, buf, (size_t)size); workSize = (size_t)size; }
    if (encryptFlag && workSize > 0u) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(workBuf); return -6; }
        xor_cipher(workBuf, workSize, key, sizeof key);
    }
    if (n->entry.data) free(n->entry.data);
    n->entry.data = NULL;
    if (workSize > 0u) {
        n->entry.data = (unsigned char*)malloc(workSize);
        if (!n->entry.data) { free(workBuf); return -8; }
        memcpy(n->entry.data, workBuf, workSize);
    }
    if (newTitle && *newTitle) { strncpy(n->entry.title, newTitle, MAX_TITLE-1); n->entry.title[MAX_TITLE-1]='\0'; }
    n->entry.originalSize = (unsigned long)size;
    n->entry.storedSize = (unsigned long)workSize;
    n->entry.flags = (compressFlag?FLAG_COMPRESSED:0u) | (encryptFlag?FLAG_ENCRYPTED:0u);
    n->entry.hash = hash32;
    n->entry.isPublic = makePublic ? 1 : 0;
    free(workBuf);
    return 0;
}

int lockerGetContent(const char *title, unsigned char **outBuf, unsigned long *outSize) {
    indexNode_t *n;
    unsigned char *buf;
    size_t nbytes;
    unsigned char key[128];
    if (!title || !outBuf || !outSize) return -1;
    *outBuf = NULL; *outSize = 0;
    n = findNode(title, NULL);
    if (!n) return -2;
    if (g_role == ROLE_PUBLIC && !n->entry.isPublic) return -3;
    nbytes = (size_t)n->entry.storedSize;
    if (nbytes == 0) { *outBuf = NULL; *outSize = 0; return 0; }
    buf = (unsigned char*)malloc(nbytes);
    if (!buf) return -4;
    memcpy(buf, n->entry.data, nbytes);
    if (n->entry.flags & FLAG_ENCRYPTED) {
        if (derive_key(g_masterPin, key, sizeof key) == 0) { free(buf); return -5; }
        xor_cipher(buf, nbytes, key, sizeof key);
    }
    if (n->entry.flags & FLAG_COMPRESSED) {
        unsigned char *tmp;
        size_t outN;
        tmp = (unsigned char*)malloc((size_t)n->entry.originalSize);
        if (!tmp) { free(buf); return -6; }
        outN = rle_decompress(buf, nbytes, tmp, (size_t)n->entry.originalSize);
        free(buf);
        if (outN != (size_t)n->entry.originalSize) { free(tmp); return -7; }
        buf = tmp; nbytes = outN;
    }
    /* Optional integrity check */
    if (n->entry.originalSize > 0) {
        unsigned int calc = (unsigned int)compute_file_hash(buf, (size_t)n->entry.originalSize);
        if (n->entry.hash != 0u && calc != n->entry.hash) { free(buf); return -9; }
    }
    *outBuf = buf; *outSize = (unsigned long)nbytes;
    return 0;
}