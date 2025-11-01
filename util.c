/* util.c - small helpers (file IO, timestamp, debug) */

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int g_runtimeDebug = 0; /* runtime-controlled debug printing */

void dbg(const char *fmt, ...) {
#ifdef DEBUG
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#else
    if (g_runtimeDebug) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    } else {
        (void)fmt;
    }
#endif
}

unsigned long util_timestamp(void) {
    static unsigned long fake = 0;
    return ++fake;
}

int util_readFile(const char *path, unsigned char **buffer, size_t *size) {
    FILE *f;
    long len;
    unsigned char *buf;
    if (!path || !buffer || !size) return -1;
    f = fopen(path, "rb");
    if (!f) return -2;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -3; }
    len = ftell(f);
    if (len < 0) { fclose(f); return -4; }
    rewind(f);
    buf = (unsigned char*)malloc((size_t)len);
    if (!buf) { fclose(f); return -5; }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) { free(buf); fclose(f); return -6; }
    fclose(f);
    *buffer = buf;
    *size = (size_t)len;
    return 0;
}

int util_writeFile(const char *path, const unsigned char *buffer, size_t size) {
    FILE *f;
    /* Allow size==0 with NULL buffer to create an empty file */
    if (!path || (!buffer && size > 0)) return -1;
    f = fopen(path, "wb");
    if (!f) return -2;
    if (size > 0) {
        if (fwrite(buffer, 1, size, f) != size) { fclose(f); return -3; }
    }
    fclose(f);
    return 0;
}

int util_ensureStorageDir(void) {
    /* Best effort: on POSIX/macOS, create storage directory if missing. */
    /* Using standard C system() from stdlib.h, allowed by assignment constraints. */
    int rc;
    rc = system("mkdir -p storage > /dev/null 2>&1");
    (void)rc;
    return 1;
}
