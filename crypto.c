/*
 * crypto.c - Cryptography Module Implementation
 * Author: Edward Doan
 */

#include "crypto.h"

/* Simple LCG PRNG - used for utility byte generation */
static unsigned long prng_state = 1;
#define PRNG_A 1103515245UL
#define PRNG_C 12345UL
#define PRNG_M 2147483648UL

void prng_seed(unsigned long seed) {
    prng_state = seed % PRNG_M;
    if (prng_state == 0) prng_state = 1;
}

unsigned long prng_next(void) {
    prng_state = (PRNG_A * prng_state + PRNG_C) % PRNG_M;
    return prng_state;
}

unsigned char prng_byte(void) {
    return (unsigned char)(prng_next() & 0xFF);
}

/* Key derivation: simple expansion of PIN into byte sequence.
 * Not cryptographically secure but fine for assignment/demo. */
size_t derive_key(const char *pin, unsigned char *out, size_t maxLen) {
    unsigned char acc;
    size_t pinLen, i;
    if (!pin || !out || maxLen == 0) return 0;
    pinLen = strlen(pin);
    if (pinLen == 0) return 0;
    acc = 0xA5;
    for (i = 0; i < maxLen; i++) {
        acc ^= (unsigned char)pin[i % pinLen];
        acc = (unsigned char)((acc << 3) | (acc >> 5));
        out[i] = acc;
    }
    return maxLen;
}

/* DJB2 for PIN hashing (used by verify_pin) */
unsigned long hash_pin(const char *pin) {
    unsigned long hash = 5381UL;
    size_t i, len;
    if (!pin) return 0;
    len = strlen(pin);
    for (i = 0; i < len; i++) hash = ((hash << 5) + hash) + (unsigned char)pin[i];
    return hash;
}

/* XOR cipher (symmetric) */
void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen) {
    size_t i;
    if (!data || !key || keyLen == 0) return;
    for (i = 0; i < n; i++) data[i] ^= key[i % keyLen];
}

int encrypt_data(unsigned char *data, size_t n, const char *pin) {
    unsigned char key[256]; size_t keyLen;
    if (!data || !pin || n == 0) return -1;
    keyLen = (n < sizeof key) ? n : sizeof key;
    if (derive_key(pin, key, keyLen) == 0) return -1;
    xor_cipher(data, n, key, keyLen);
    return 0;
}

int decrypt_data(unsigned char *data, size_t n, const char *pin) {
    return encrypt_data(data, n, pin); /* symmetric */
}

unsigned long compute_file_hash(const unsigned char *data, size_t n) {
    unsigned long hash = 2166136261UL; size_t i;
    if (!data || n == 0) return 0;
    for (i = 0; i < n; i++) { hash ^= (unsigned long)data[i]; hash *= 16777619UL; }
    return hash;
}

int verify_file_integrity(const unsigned char *data, size_t n, unsigned long storedHash) {
    if (!data) return 0;
    return (compute_file_hash(data, n) == storedHash) ? 1 : 0;
}

int verify_pin(const char *pin, unsigned long storedHash) {
    if (!pin) return 0;
    return (hash_pin(pin) == storedHash) ? 1 : 0;
}
