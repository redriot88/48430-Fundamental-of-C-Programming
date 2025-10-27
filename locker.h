/* locker.h
 * Public API for the Personal Document Safe Locker (ANSI-C friendly)
 */

#ifndef LOCKER_H
#define LOCKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCKER_MAGIC 0x4C4B5255u
#define LOCKER_VERSION 1

#define MAX_TITLE 128
#define MAX_PIN 32

/* Roles */
#define ROLE_ADMIN 1
#define ROLE_PUBLIC 2

/* Flags */
#define FLAG_COMPRESSED (1u<<0)
#define FLAG_ENCRYPTED  (1u<<1)

typedef struct {
    char title[MAX_TITLE];
    unsigned long originalSize;
    unsigned long storedSize;
    unsigned int flags;
    int isPublic;
    unsigned char *data;
} indexEntry_t;

typedef struct indexNode {
    indexEntry_t entry;
    struct indexNode *next;
} indexNode_t;

typedef struct {
    indexNode_t *head;
    int count;
} index_t;

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
} lockerHeader_t;

/* Debug function: implemented in util.c. Use DBG(...) in code which maps to dbg. */
void dbg(const char *fmt, ...);
#define DBG dbg

/* API */
index_t *lockerGetIndex(void);
int lockerGetRole(void);

int lockerOpen(const char *lockerPath, const char *pin);
int lockerClose(void);

int lockerChangePIN(const char *oldPin, const char *newPin);

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

void lockerList(void);
int lockerSearch(const char *pattern);

int lockerSaveIndex(void);
int lockerLoadIndex(void);

void printMenu(void);

#endif /* LOCKER_H */
/* locker.h
 * Public API for the Personal Document Safe Locker (clean, single copy).
 */

#ifndef LOCKER_H
#define LOCKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCKER_MAGIC 0x4C4B5255u
#define LOCKER_VERSION 1

#define MAX_TITLE 128
#define MAX_PIN 32

/* Roles */
#define ROLE_ADMIN 1
#define ROLE_PUBLIC 2

/* Flags */
#define FLAG_COMPRESSED (1u<<0)
#define FLAG_ENCRYPTED  (1u<<1)

typedef struct {
    char title[MAX_TITLE];
    unsigned long originalSize;
    unsigned long storedSize;
    unsigned int flags;
    int isPublic;
    unsigned char *data;
} indexEntry_t;

typedef struct indexNode {
    indexEntry_t entry;
    struct indexNode *next;
} indexNode_t;

typedef struct {
    indexNode_t *head;
    int count;
} index_t;

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
} lockerHeader_t;

/* API */
index_t *lockerGetIndex(void);
int lockerGetRole(void);

int lockerOpen(const char *lockerPath, const char *pin);
int lockerClose(void);
int lockerChangePIN(const char *oldPin, const char *newPin);

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

void lockerList(void);
int lockerSearch(const char *pattern);

int lockerSaveIndex(void);
int lockerLoadIndex(void);

void printMenu(void);

#ifdef DEBUG
#define DBG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DBG(...) ((void)0)
#endif

#endif /* LOCKER_H */
/*
 * locker.h
 * Public API for the Personal Document Safe Locker.
 *
 * This header exposes a lightweight API for a small in-memory locker.
 * The implementation in `locker.c` uses a singly-linked list for entries
 * (simpler to implement and safer for insert/remove in an educational
 * setting than a hand-rolled dynamic array).
 *
 * Data shapes:
 *  - indexEntry_t: metadata + pointer to in-memory data buffer
 *  - indexNode_t: linked-list node containing an indexEntry_t
 *  - index_t: simple container with head pointer and count
 *
 * Note: persistence functions are stubbed for the assignment; students are
 * expected to implement a binary on-disk format (header + entries + data)
 * if required.
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

/* Single entry metadata structure. `data` points to in-memory bytes.
 * Students: storing file offsets and persisting data is left as an exercise. */
typedef struct {
    char title[MAX_TITLE];          /* logical title / filename */
    unsigned long originalSize;     /* size before compression/encryption */
    unsigned long storedSize;       /* size stored in locker */
    unsigned int flags;             /* compression/encryption flags */
    int isPublic;                   /* 1 if visible to public */
    unsigned char *data;            /* in-memory stored bytes (heap allocated)
                                     * Free when removing entry. */
} indexEntry_t;

/* Linked list node wrapping an entry */
typedef struct indexNode {
    indexEntry_t entry;
    struct indexNode *next;
} indexNode_t;

/* Index container: head pointer and count */
typedef struct {
    indexNode_t *head;
    int count;
} index_t;

/* Locker file header format (includes PIN) */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
/*
 * locker.h
 * Public API for the Personal Document Safe Locker.
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
    char title[MAX_TITLE];
    unsigned long originalSize;
    unsigned long storedSize;
    unsigned int flags;
    int isPublic;
    unsigned char *data;
} indexEntry_t;

typedef struct indexNode {
    indexEntry_t entry;
    struct indexNode *next;
} indexNode_t;

typedef struct {
    indexNode_t *head;
    int count;
} index_t;

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
} lockerHeader_t;

index_t *lockerGetIndex(void);
int lockerGetRole(void);

int lockerOpen(const char *lockerPath, const char *pin);
int lockerClose(void);

int lockerChangePIN(const char *oldPin, const char *newPin);

int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

void lockerList(void);
int lockerSearch(const char *pattern);

int lockerSaveIndex(void);
int lockerLoadIndex(void);

void printMenu(void);

#ifdef DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0)
#else
#define DBG(fmt, ...) ((void)0)
#endif

#endif /* LOCKER_H */

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

/* Single entry metadata structure. `data` points to in-memory bytes.
 * Students: storing file offsets and persisting data is left as an exercise. */
typedef struct {
    char title[MAX_TITLE];          /* logical title / filename */
    unsigned long originalSize;     /* size before compression/encryption */
    unsigned long storedSize;       /* size stored in locker */
    unsigned int flags;             /* compression/encryption flags */
    int isPublic;                   /* 1 if visible to public */
    unsigned char *data;            /* in-memory stored bytes (heap allocated)
                                     * Free when removing entry. */
} indexEntry_t;

/* Linked list node wrapping an entry */
typedef struct indexNode {
    indexEntry_t entry;
    struct indexNode *next;
} indexNode_t;

/* Index container: head pointer and count */
typedef struct {
    indexNode_t *head;
    int count;
} index_t;

/* Locker file header format (includes PIN) */
typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int count;
    char masterPin[MAX_PIN];
} lockerHeader_t;

/* Accessors for testing/inspection */
index_t *lockerGetIndex(void);
int lockerGetRole(void);

/* Lifecycle */
int lockerOpen(const char *lockerPath, const char *pin);
int lockerClose(void);

/* Master PIN */
int lockerChangePIN(const char *oldPin, const char *newPin);

/* Core operations (return 0 success, non-zero error) */
int lockerAddFile(const char *filepath, const char *title, int compressFlag, int encryptFlag, int makePublic);
int lockerExtractFile(const char *title, const char *outputPath);
int lockerRemoveFile(const char *title);

/* Listing & Search */
void lockerList(void);
int lockerSearch(const char *pattern);

/* Index persistence (stubs) */
int lockerSaveIndex(void);
int lockerLoadIndex(void);

/* Menu (interactive mode) */
void printMenu(void);

/* Debug macro */
#ifdef DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0)
#else
#define DBG(fmt, ...) ((void)0)
#endif

#endif /* LOCKER_H */
