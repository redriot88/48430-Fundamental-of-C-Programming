/*******************************************************************************
 * 48430 Fundamentals of C Programming - Assignment 3 Group Project
 * Name: Alex, Ed, Edward, Tarun.
 * Student ID: [Alex SID], 14386371, [SID rest]
 * Date of submission: 4/10/25
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include locker.c
/*******************************************************************************
Main
*******************************************************************************/
int main(void) {
    int choice = 0;
    char input[10];

  /*
  if (!checkMasterPIN()){
    printf("Access denied.\n");
    return 1;
  */
  

    while (1) {
        printMenu();
        printf("Enter your PIN>\n"); /*either PIN or we can have no ask for masterpassword*/
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            return 0; /*or we can use continue i.e.
            printf("Invalid input.\n");
            continue;
            */
        }
        (void)getchar(); /* clear newline */
        /*
        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0 {
        printf("Exiting Personal Document Locker.\n");
        break;
        */

        /* 
        choice = atoi(input); 
        */

        switch (choice) {
            case 1: addFile(); break;
            case 2: extractFile(); break;
            case 3: removeFile(); break;
            case 4: listFile(); break;
            case 5: searchFile(); break;
            case 6: changePIN(); break;
            case 7: quit(); return 0; /* we'll need atoi and read in q for 7 to quit, but case 7 may be redundant*/
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}
