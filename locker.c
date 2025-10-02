/*************************************************************************************
Typedefs & Structs Prototypes / List Preprocessing Directives
**************************************************************************************/
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include "locker.h"

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

/*************************************************************************************
Menu & Helper Prototypes
**************************************************************************************/
void printMenu(void) {
    printf("\nPersonal Document Locker\n"
           "1. Add file\n"
           "2. Extract file\n"
           "3. List files\n"
           "4. Search by filename\n"
           "5. Remove file\n"
           "6. Change master PIN\n")
           "q. Quit\n";
  printf("--------------------------");
  printf("Enter your choice: ");
}

/*******************************************************************************
Core Functionality
*******************************************************************************/

/*
static char masterPIN[20] = "admin";

int checkMasterPIN(void){
  char input[20];
  printf(scanf "%19s", input) != 1) return 0;
  if (strcmp(input, masterPIN) == 0) {
    return 1;
  }
  return 0;
}
*/

/*
void addFile(void){
  printf("Add File Function\n");
  TODO: implement compression, encryption, storage.
}
*/

/*
void extractFile(void){
  printf("Extract File Function\n");
  TODO: implement decryption and decompression
}
*/

/*
void listFiles(void){
  printf("Search File Function");
  TODO string matching filename
}
  */

/*
void removeFile(void){
  printf("Remove File Function\n");
  TODO: delete file entry from storage
}
*/

/*
void changeMasterPIN(void){
  char newPIN[20];
  printf("Enter new master PIN: ");
  if (scanf("19%s", newPIN) == 1){
    strncpy(masterPIN, newPIN, sizeof(masterPIN) - 1);
    masterPIN[sizeof(masterPIN) - 1] = "\0";
    printf("Password changed successfully.\n");
  }
}
*/
