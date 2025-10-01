/*************************************************************************************
Typedefs & Structs Prototypes
**************************************************************************************/

#ifndef LOCKER_H
#define LOCKER_H

#include <stdlib.h>
/* #include <stdio.h> // don't need need this here*/
/* #include <string.h> // don't need this here*/

#define MAX_FILENAME 256

typedef struct {
  char filename[MAX_FILENAME]; /*original file name*/
  unsigned long originalSize; /*original size in bytes*/
  unsigned long storedSize; /*compressed and encrypted size*/
  unsigned timestamp; /* epoch, not sure if we need this...*/
  int compressed; /*compressed indicator, 1 if compressed, 0 otherwise*/
  /* unsigned long fileOffset; //offset within storage but not sure if we need */
} indexEntry_t;

/*if we need index struture to be stored in memory as a dynamic array*/
typedef struct {
  indexEntry_t *entries;
  int count;
  int capacity;
} index_t;

/*************************************************************************************
Function Prototypes; this is just a draft
**************************************************************************************/
/*storage.c or storage.h*/
void lockerOpen(void);
void lockerClose(void);

/*Index Operations*/
void indexLoad(void);
void indexSave(void);

/*Main Operations*/
void addFile(void); /*add file to the locker*/
void extractFile(void); /*extract file from the locker*/
void removeFile(void); /*removes file from the locker*/
void listFile(void); /*lists files within locker*/
void searchFile(void);/*returns list or matches*/
void changePIN(void); /*changes master PIN or password*/

/*************************************************************************************
Menu & Helper Prototypes
**************************************************************************************/
void printMenu(void) {
    printf("\nLibrary Management System\n"
           "1. Add file\n"
           "2. Extract file\n"
           "3. List files\n"
           "4. Search by filename\n"
           "5. Remove file\n"
           "6. Change master PIN\n")
           "q. Quit\n";
}



endif
