#include "crypto.h"

size_t derive_key(const char *pin, unsigned char *out, size_t maxLen) {
    if (!pin || !out || maxLen==0) return 0;
    /* simple expansion: repeat pin chars with rolling xor */
    unsigned char acc = 0xA5;
    size_t pinLen = strlen(pin);
    if (pinLen == 0) return 0;
    for (size_t i=0;i<maxLen;i++) {
        acc ^= (unsigned char)pin[i % pinLen];
        acc = (unsigned char)((acc << 3) | (acc >> 5)); /* rotate */
        out[i] = acc;        
    }
    return maxLen;
}

void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen) {
    if (!data || !key || keyLen==0) return;
    for (size_t i=0;i<n;i++) data[i] ^= key[i % keyLen];
}
