/**
 * 48430 Fundamentals of C Programming - Assignment 3 Group Project
 * Personal Document Safe Locker (Interactive driver)
 * Group: grp08  Lab: 2pm
 *
 * This driver provides two runtime modes:
 *  - Interactive: admin/public login and menu-driven operations (add/extract/list/...)
 *  - CLI tool: `encrypt` minimal demo to compress+encrypt a file for extra marks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "locker.h"
#include "compress.h"
#include "crypto.h"
#include "util.h"

static void consumeLine(void) {
  int c; while ((c=getchar())!='\n' && c!=EOF) { /* discard */ }
}

/* Minimal demo: compress+encrypt an input file to output file using optional PIN */
static int encrypt_demo(const char *inpath, const char *outpath, const char *pin) {
  unsigned char *inbuf = NULL;
  size_t inSize = 0;
  unsigned char *work = NULL;
  size_t workCap, workSize;
  unsigned char key[128];
  size_t keyLen;
  int rc;

  if (!inpath || !outpath) return -1;
  rc = util_readFile(inpath, &inbuf, &inSize);
  if (rc != 0) return rc;
  /* allocate worst-case for RLE */
  workCap = inSize * 2 + 4;
  work = (unsigned char*)malloc(workCap);
  if (!work) { free(inbuf); return -2; }
  workSize = rle_compress(inbuf, inSize, work, workCap);
  if (workSize == 0) { /* no compression => store raw */
    memcpy(work, inbuf, inSize);
    workSize = inSize;
  }
  if (pin && *pin) {
    keyLen = sizeof key;
    if (derive_key(pin, key, keyLen) == 0) { free(inbuf); free(work); return -3; }
    xor_cipher(work, workSize, key, keyLen);
  }
  rc = util_writeFile(outpath, work, workSize);
  free(inbuf); free(work);
  return rc;
}

int main(int argc, char **argv) {
  /* CLI mini-tools: support `encrypt` mode for demo: ./program.out encrypt inpath outpath [pin] */
  if (argc >= 2 && strcmp(argv[1], "encrypt") == 0) {
    const char *pin;
    if (argc < 4) {
      fprintf(stderr, "Usage: %s encrypt <input> <output> [pin]\n", argv[0]);
      return 1;
    }
    pin = (argc >= 5) ? argv[4] : "admin";
    int r = encrypt_demo(argv[2], argv[3], pin);
    if (r != 0) {
      fprintf(stderr, "encrypt failed (%d)\n", r);
      return 1;
    }
    printf("Encrypted+compressed %s -> %s\n", argv[2], argv[3]);
    return 0;
  }

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
        lockerClose();
        break; /* back to login loop */
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
