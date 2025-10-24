#include "compress.h"

size_t rle_compress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap) {
    size_t oi;
    size_t i;
    if (!in || !out) return 0;
    oi = 0;
    i = 0;
    while (i < n) {
        unsigned char b;
        size_t run;
        b = in[i];
        run = 1;
        while (i + run < n && in[i + run] == b && run < 255) {
            run++;
        }
        if (oi + 2 > outCap) return 0; /* no room */
        out[oi++] = (unsigned char)run;
        out[oi++] = b;
        i += run;
    }
    return oi;
}

size_t rle_decompress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap) {
    size_t oi;
    size_t i;
    if (!in || !out) return 0;
    oi = 0;
    i = 0;
    while (i + 1 < n) {
        unsigned char count;
        unsigned char byte;
        unsigned char k;
        count = in[i];
        byte  = in[i + 1];
        if (oi + count > outCap) return 0;
        k = 0;
        while (k < count) {
            out[oi++] = byte;
            k++;
        }
        i += 2;
    }
    return oi;
}
