/**
 * 48430 Fundamentals of C Programming - Assignment 3 Group Project
 * Personal Document Safe Locker (Interactive driver)
 * Group: (ADD GROUP NUMBER)  Lab: (ADD LAB NUMBER)
 * Build:
 *   gcc -std=c11 -Wall -Wextra -pedantic -ansi -DDEBUG -o locker \
 *       main.c locker.c
 * (Later add: storage.c compress.c crypto.c util.c)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "locker.h"

static void consumeLine(void) {
  int c; while ((c=getchar())!='\n' && c!=EOF) { /* discard */ }
}

int main(void) {
  for (;;) {
    int roleChoice;
    char pin[64];
    int choice;
    printf("Login as:\n1. Admin\n2. Public\nSelect: ");
    if (scanf("%d", &roleChoice) != 1) return 1; consumeLine();
    if (roleChoice == 1) {
      printf("Enter admin PIN: "); if (!fgets(pin, sizeof pin, stdin)) return 1; pin[strcspn(pin, "\n")] = 0;
      if (lockerOpen("locker.dat", pin) != 0) {
        printf("Failed to open locker (wrong PIN?)\n");
        continue; /* back to login */
      }
    } else {
      if (lockerOpen("locker.dat", NULL) != 0) { /* public mode */
        printf("Failed to open locker\n");
        continue; /* back to login */
      }
    }
    /* inner menu loop */
    for (;;) {
      printMenu();
      if (scanf("%d", &choice) != 1) { printf("Exiting.\n"); lockerClose(); return 0; }
      consumeLine();
      if (choice == 7) { /* logout */
        printf("Logged out.\n");
        break; /* back to login loop, keep in-memory entries */
      }
      if (choice == 8) { /* quit */
        printf("Goodbye.\n");
        lockerClose();
        return 0;
      }
      if (choice == 1) {
        char path[256], title[128];
        char ans[8];
        printf("Path to file: "); if (!fgets(path, sizeof path, stdin)) continue; path[strcspn(path,"\n")] = 0;
        printf("Title to store: "); if (!fgets(title, sizeof title, stdin)) continue; title[strcspn(title,"\n")] = 0;
        printf("Make public? (y/n): "); if (!fgets(ans, sizeof ans, stdin)) ans[0] = 'n';
        if (lockerAddFile(path, title, 1, 1, (ans[0]=='y'||ans[0]=='Y'))==0) printf("Added %s\n", title); else printf("Add failed (admin only or error)\n");
      } else if (choice == 2) {
        char title[128], out[256];
        printf("Title to extract: "); if (!fgets(title,sizeof title,stdin)) continue; title[strcspn(title,"\n")] = 0;
        printf("Output path: "); if (!fgets(out,sizeof out,stdin)) continue; out[strcspn(out,"\n")] = 0;
        if (lockerExtractFile(title, out)==0) printf("Extracted to %s\n", out); else printf("Extract failed\n");
      } else if (choice == 3) {
        char title[128];
        printf("Title to remove: "); if (!fgets(title,sizeof title,stdin)) continue; title[strcspn(title,"\n")] = 0;
        if (lockerRemoveFile(title)==0) printf("Removed %s\n", title); else printf("Remove failed\n");
      } else if (choice == 4) {
        lockerList();
      } else if (choice == 5) {
        char pattern[128];
        int m;
        printf("Search pattern: "); if (!fgets(pattern,sizeof pattern,stdin)) continue; pattern[strcspn(pattern,"\n")] = 0;
        m = lockerSearch(pattern);
        printf("%d match(es).\n", m);
      } else if (choice == 6) {
        char oldPin[64], newPin[64];
        printf("Old PIN: "); if (!fgets(oldPin,sizeof oldPin,stdin)) continue; oldPin[strcspn(oldPin,"\n")] = 0;
        printf("New PIN: "); if (!fgets(newPin,sizeof newPin,stdin)) continue; newPin[strcspn(newPin,"\n")] = 0;
        if (lockerChangePIN(oldPin,newPin)==0) printf("PIN changed.\n"); else printf("PIN change failed.\n");
      } else {
        printf("Invalid choice.\n");
      }
    }
  }
}
