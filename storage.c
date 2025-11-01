/* storage.c
 * Clean ANSI-C implementation for locker persistence.
 */

#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define STORAGE_MAGIC 0x4C434B52U /* 'L' 'C' 'K' 'R' */
#define STORAGE_VERSION 2

static int write_u32(FILE *f, unsigned int v) {
    return fwrite(&v, sizeof(v), 1, f) == 1 ? 0 : -1;
}

static int read_u32(FILE *f, unsigned int *out) {
    return fread(out, sizeof(*out), 1, f) == 1 ? 0 : -1;
}

int storageSaveAll(const char *path, const index_t *idx, const char *masterPin) {
    FILE *f;
    unsigned int magic;
    unsigned int count;
    unsigned int pinLen;
    indexNode_t *n;

    if (!path || !idx) return -1;
    f = fopen(path, "wb");
    if (!f) return -1;

    magic = STORAGE_MAGIC;
    if (fwrite(&magic, sizeof(magic), 1, f) != 1) goto err;
    if (write_u32(f, STORAGE_VERSION) != 0) goto err;
    count = (unsigned int)idx->count;
    if (write_u32(f, count) != 0) goto err;

    pinLen = masterPin ? (unsigned int)strlen(masterPin) : 0u;
    if (write_u32(f, pinLen) != 0) goto err;
    if (pinLen > 0u) {
        if (fwrite(masterPin, 1, pinLen, f) != pinLen) goto err;
    }

    n = idx->head;
    while (n) {
        indexEntry_t *e = &n->entry;
        unsigned int titleLen = (unsigned int)strlen(e->title);
        unsigned int hash = e->hash;
        if (write_u32(f, titleLen) != 0) goto err;
        if (titleLen > 0u && fwrite(e->title, 1, titleLen, f) != titleLen) goto err;
        if (write_u32(f, (unsigned int)e->originalSize) != 0) goto err;
        if (write_u32(f, (unsigned int)e->storedSize) != 0) goto err;
        if (write_u32(f, hash) != 0) goto err;
        /* pack flags (low 7 bits) and isPublic in high bit */
        unsigned char meta;
        meta = (unsigned char)(e->flags & 0x7Fu);
        if (e->isPublic) meta |= 0x80u;
        if (fwrite(&meta, 1, 1, f) != 1) goto err;
        if (e->storedSize > 0u) {
            if (fwrite(e->data, 1, (size_t)e->storedSize, f) != (size_t)e->storedSize) goto err;
        }
        n = n->next;
    }

    fclose(f);
    return 0;
err:
    fclose(f);
    return -1;
}

int storageLoadAll(const char *path, index_t *idx, char *outMasterPin, size_t maxPinLen) {
    FILE *f;
    unsigned int magic = 0u;
    unsigned int version = 0u;
    unsigned int file_count = 0u;
    unsigned int pinLen = 0u;
    unsigned int i;

    if (!path || !idx) return -1;
    f = fopen(path, "rb");
    if (!f) return -1;

    if (fread(&magic, sizeof(magic), 1, f) != 1) goto err;
    if (magic != STORAGE_MAGIC) goto err;

    if (read_u32(f, &version) != 0) goto err;
    if (version != 1u && version != 2u) goto err;

    if (read_u32(f, &file_count) != 0) goto err;
    if (read_u32(f, &pinLen) != 0) goto err;

    if (pinLen > 0u && outMasterPin && maxPinLen > 0u) {
        unsigned int toRead = pinLen < (unsigned int)(maxPinLen - 1u) ? pinLen : (unsigned int)(maxPinLen - 1u);
        if (fread(outMasterPin, 1, toRead, f) != toRead) goto err;
        outMasterPin[toRead] = '\0';
        if (pinLen > toRead) {
            if (fseek(f, (long)(pinLen - toRead), SEEK_CUR) != 0) goto err;
        }
    } else if (pinLen > 0u) {
        if (fseek(f, (long)pinLen, SEEK_CUR) != 0) goto err;
    } else if (outMasterPin && maxPinLen > 0u) {
        outMasterPin[0] = '\0';
    }

    /* free existing index nodes */
    while (idx->head) {
        indexNode_t *tmp = idx->head;
        idx->head = tmp->next;
        if (tmp->entry.data) free(tmp->entry.data);
        free(tmp);
    }
    idx->count = 0;

    for (i = 0u; i < file_count; ++i) {
    unsigned int titleLen = 0u;
        char *title = NULL;
        unsigned int originalSize = 0u;
        unsigned int storedSize = 0u;
    unsigned int hash = 0u;
        unsigned char flags = 0u;
        indexEntry_t entry;

        if (read_u32(f, &titleLen) != 0) goto err;
        title = (char*)calloc(1, (size_t)titleLen + 1u);
        if (titleLen > 0u) {
            if (fread(title, 1, (size_t)titleLen, f) != (size_t)titleLen) { free(title); goto err; }
        }

        if (read_u32(f, &originalSize) != 0) { free(title); goto err; }
        if (read_u32(f, &storedSize) != 0) { free(title); goto err; }
        if (version >= 2u) {
            if (read_u32(f, &hash) != 0) { free(title); goto err; }
        } else {
            hash = 0u; /* legacy files have no stored hash */
        }
        if (fread(&flags, 1, 1, f) != 1) { free(title); goto err; }

        memset(&entry, 0, sizeof(entry));
        strncpy(entry.title, title, sizeof(entry.title) - 1u);
        entry.originalSize = (unsigned long)originalSize;
        entry.storedSize = (unsigned long)storedSize;
        entry.flags = (unsigned int)(flags & 0x7Fu);
        entry.hash = hash;
        entry.isPublic = (flags & 0x80u) ? 1 : 0;
        entry.data = NULL;
        if (storedSize > 0u) {
            entry.data = malloc((size_t)storedSize);
            if (!entry.data) { free(title); goto err; }
            if (fread(entry.data, 1, (size_t)storedSize, f) != (size_t)storedSize) { free(entry.data); free(title); goto err; }
        }

        /* append to index (push front) */
        {
            indexNode_t *node = (indexNode_t*)malloc(sizeof(indexNode_t));
            if (!node) { if (entry.data) free(entry.data); free(title); goto err; }
            node->entry = entry;
            node->next = idx->head;
            idx->head = node;
            idx->count++;
        }

        free(title);
    }

    fclose(f);
    return 0;
err:
    fclose(f);
    return -1;
}
