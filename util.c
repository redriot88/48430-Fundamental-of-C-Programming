#include "util.h"

unsigned long util_timestamp(void) {
    static unsigned long fake = 0;
    return ++fake;
}

int util_readFile(const char *path, unsigned char **buffer, size_t *size) {
    if (!path || !buffer || !size) return -1;
    FILE *f = fopen(path, "rb");
    if (!f) return -2;
    if (fseek(f, 0, SEEK_END)!=0) { fclose(f); return -3; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return -4; }
    rewind(f);
    unsigned char *buf = (unsigned char*)malloc((size_t)len);
    if (!buf) { fclose(f); return -5; }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) { free(buf); fclose(f); return -6; }
    fclose(f);
    *buffer = buf;
    *size = (size_t)len;
    return 0;
}

int util_writeFile(const char *path, const unsigned char *buffer, size_t size) {
    if (!path || !buffer) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -2;
    if (fwrite(buffer, 1, size, f) != size) { fclose(f); return -3; }
    fclose(f);
    return 0;
}
