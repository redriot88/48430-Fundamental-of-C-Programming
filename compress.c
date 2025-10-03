#include "compress.h"

size_t rle_compress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap) {
    if (!in || !out) return 0;
    size_t oi = 0;
    for (size_t i=0;i<n;) {
        unsigned char b = in[i];
        size_t run = 1;
        while (i+run < n && in[i+run]==b && run < 255) run++;
        if (oi + 2 > outCap) return 0; /* no room */
        out[oi++] = (unsigned char)run;
        out[oi++] = b;
        i += run;
    }
    return oi;
}

size_t rle_decompress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap) {
    if (!in || !out) return 0;
    size_t oi = 0;
    for (size_t i=0;i+1<n; i+=2) {
        unsigned char count = in[i];
        unsigned char byte  = in[i+1];
        if (oi + count > outCap) return 0;
        for (unsigned char k=0;k<count;k++) out[oi++] = byte;
    }
    return oi;
}
