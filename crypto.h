#ifndef CRYPTO_H
#define CRYPTO_H
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Derive a pseudo key stream from PIN (very simple hash/expansion) */
size_t derive_key(const char *pin, unsigned char *out, size_t maxLen);

void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen);

#endif /* CRYPTO_H */
