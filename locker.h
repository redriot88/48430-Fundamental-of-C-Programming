/**
 * Personal Document Safe Locker - Public API (Checkpoint 1 Skeleton)
 * Group: (ADD GROUP NUMBER)  Lab: (ADD LAB NUMBER)
 * Build (example):
 *   gcc -std=c11 -Wall -Wextra -pedantic -ansi -DDEBUG -o locker \
 *       main.c locker.c storage.c crypto.c compress.c util.c
 * Allowed standard libs only: stdio.h, stdlib.h, string.h, math.h
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

/* Roles */
#define ROLE_ADMIN  1
#define ROLE_PUBLIC 2

/* Flag bits */
#define FLAG_COMPRESSED  (1u<<0)
#define FLAG_ENCRYPTED   (1u<<1)

typedef struct {
    char title[MAX_TITLE];          /* logical title / filename */
    unsigned long originalSize;     /* size before compression/encryption */
    unsigned long storedSize;       /* size stored in locker */
    unsigned long dataOffset;       /* offset inside locker file */
    unsigned int flags;             /* compression/encryption flags */
    int isPublic;                   /* 1 if visible to public */
    unsigned char *data;            /* in-memory stored bytes (no persistence yet) */
} indexEntry_t;

typedef struct {
    indexEntry_t *entries;
    int count;
    int capacity;
} index_t;

/* Accessor for global index (implemented in locker.c) */
index_t *lockerGetIndex(void);
int lockerGetRole(void);

/* Lifecycle */
int lockerOpen(const char *lockerPath, const char *pin); /* returns 0 on success */
int lockerClose(void);

/* Master PIN */
int lockerChangePIN(const char *oldPin, const char *newPin);

/* Core operations (return 0 success, non-zero error) */
int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

/* Listing & Search */
void lockerList(void);
int lockerSearch(const char *pattern); /* returns matches count */

/* Index persistence */
int lockerSaveIndex(void);
int lockerLoadIndex(void);

/* Menu (interactive mode) */
void printMenu(void);

/* Debug macro (ANSI C90 compatible: no variadic macros) */
#ifdef DEBUG
#define DBG1(args) do { fprintf args; } while(0)
#else
#define DBG1(args) ((void)0)
#endif

#endif /* LOCKER_H */
