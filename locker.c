/*************************************************************************************
Typedefs & Structs Prototypes
**************************************************************************************/
/*
#ifndef LOCKER_H
#define LOCKER_H
*/

/*******************************************************************************
List Preprocessing Directives
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>

#define MAX_FILENAME 256

/*******************************************************************************
List structs
*******************************************************************************/

typedef struct {
  char filename[MAX_FILENAME]; /*original file name*/
  unsigned long originalSize; /*original size in bytes*/
  unsigned long storedSize; /*compressed and encrypted size*/
  unsigned timestamp; /* epoch, not sure if we need this...*/
  int compressed; /*compressed indicator, 1 if compressed, 0 otherwise*/
  /* unsigned long fileOffset; //offset within storage but not sure if we need */
} indexEntry_t;

/*if we need index structure to be stored in memory as a dynamic array*/
typedef struct {
  indexEntry_t *entries;
  int count;
  int capacity;
} index_t;

/*************************************************************************************
Function Prototypes. Draft
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
void quit(void); /*exits program*/

/*******************************************************************************
Main
*******************************************************************************/
int main(void) {
    int choice = 0;

    while (1) {
        printMenu();
        printf("Enter your choice>\n");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            return 0;
        }
        (void)getchar(); /* clear newline */

        switch (choice) {
            case 1: addFile(); break;
            case 2: extractFile(); break;
            case 3: removeFile(); break;
            case 4: listFile(); break;
            case 5: searchFile(); break;
            case 6: changePIN(); break;
            case 7: quit(); return 0; /* we'll need atoi and read in q for 7 to quit*/
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}

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

/*******************************************************************************
Core Functionality
*******************************************************************************/

/*
endif
*/
