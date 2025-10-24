#include "crypto.h"

size_t derive_key(const char *pin, unsigned char *out, size_t maxLen) {
    unsigned char acc;
    size_t pinLen;
    size_t i;
    if (!pin || !out || maxLen==0) return 0;
    /* simple expansion: repeat pin chars with rolling xor */
    acc = 0xA5;
    pinLen = strlen(pin);
    if (pinLen == 0) return 0;
    i = 0;
    while (i < maxLen) {
        acc ^= (unsigned char)pin[i % pinLen];
        acc = (unsigned char)((acc << 3) | (acc >> 5)); /* rotate */
        out[i] = acc;
        i++;
    }
    return maxLen;
}

void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen) {
    size_t i;
    if (!data || !key || keyLen==0) return;
    i = 0;
    while (i < n) {
        data[i] ^= key[i % keyLen];
        i++;
    }
}
