/**
 * Personal Document Safe Locker - Public API
 */
#ifndef LOCKER_H
#define LOCKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCKER_MAGIC 0x4C4B5255u /* 'L' 'K' 'R' 'U' arbitrary */
#define LOCKER_VERSION 1

#define MAX_FILENAME 256
#define MAX_TITLE    128
#define MAX_PIN      32

/* Flag bits */
#define FLAG_COMPRESSED  (1u<<0)
#define FLAG_ENCRYPTED   (1u<<1)

typedef struct {
    char title[MAX_TITLE];          /* logical title / filename */
    unsigned long originalSize;     /* size before compression/encryption */
    unsigned long storedSize;       /* size stored in locker */
    unsigned long dataOffset;       /* offset inside locker file */
    unsigned int flags;             /* compression/encryption flags */
} indexEntry_t;

typedef struct {
    indexEntry_t *entries;
    int count;
    int capacity;
} index_t;

/* Locker file header format (includes PIN) */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
} lockerHeader_t;

/* Accessor for global index (implemented in locker.c) */
index_t *lockerGetIndex(void);

/* Lifecycle */
int lockerOpen(const char *lockerPath, const char *pin);
int lockerClose(void);

/* Master PIN */
int lockerChangePIN(const char *oldPin, const char *newPin);

/* Core operations */
int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

/* Listing & Search */
void lockerList(void);
int lockerSearch(const char *pattern);

/* Index persistence */
int lockerSaveIndex(void);
int lockerLoadIndex(void);

/* Menu (interactive mode) */
void printMenu(void);

/* Debug macro (ANSI‑safe) */
#ifdef DEBUG
#define DBG printf
#else
#define DBG (void)
#endif

#endif /* LOCKER_H */
