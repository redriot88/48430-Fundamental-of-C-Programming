#ifndef COMPRESS_H
#define COMPRESS_H
/* RLE compression API (simple) */
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Compress input buffer 'in' of length n into 'out'.
 * Returns number of bytes written to out, or 0 on failure (output too small).
 * Format: <count><byte> pairs where count is a single unsigned char (1..255).
 */
size_t rle_compress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap);

/* Decompress RLE buffer. Returns decompressed size or 0 on failure */
size_t rle_decompress(const unsigned char *in, size_t n, unsigned char *out, size_t outCap);

#endif /* COMPRESS_H */
