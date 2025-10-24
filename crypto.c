/*
 * crypto.c - Cryptography Module Implementation
 * Author: Edward Doan
 */

#include "crypto.h"

static unsigned long prng_state = 1;  

#define PRNG_A 1103515245UL  
#define PRNG_C 12345UL       
#define PRNG_M 2147483648UL  

void prng_seed(unsigned long seed) {
    
    prng_state = seed % PRNG_M;
    if (prng_state == 0) {
        prng_state = 1;  
    }
}

unsigned long prng_next(void) {
    prng_state = (PRNG_A * prng_state + PRNG_C) % PRNG_M;
    return prng_state;
}

unsigned char prng_byte(void) {
    return (unsigned char)(prng_next() & 0xFF);
}


/* ========== Key Derivation ========== */

size_t derive_key(const char *pin, unsigned char *out, size_t maxLen) {
    
    unsigned char acc;
    size_t pinLen;
    size_t i;
    
    /* Input validation - check for null pointers and empty data */
    if (!pin || !out || maxLen == 0) {
        return 0;
    }
    
    acc = 0xA5;
    pinLen = strlen(pin);
    
    if (pinLen == 0) {
        return 0;  /* Can't derive key from empty PIN */
    }
    
    /* Main derivation loop: cycle through PIN chars and mix them */

    for (i = 0; i < maxLen; i++) {
        /* XOR current accumulator with next PIN character */
        acc ^= (unsigned char)pin[i % pinLen];
        
        /* Rotate bits left by 3 positions to spread the bits around */
        acc = (unsigned char)((acc << 3) | (acc >> 5));
        
        out[i] = acc;
    }
    
    return maxLen;
}

unsigned long hash_pin(const char *pin) {

    unsigned long hash;
    size_t i;
    size_t len;
    
    if (!pin) {
        return 0;
    }
    
    hash = 5381UL;  /* Starting value */
    len = strlen(pin);
    
    /* 
     * DJB2 hash algorithm (found this in a C programming forum in StackOverflow)
     * Formula: hash = hash * 33 + character
     * The 33 is a number that gives good distribution
     */
    for (i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)pin[i];  /* hash * 33 + c */
    }
    
    return hash;
}


/* ========== Encryption/Decryption Implementation ========== */

void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen) {
    size_t i;
    
    /* Check for invalid inputs */
    if (!data || !key || keyLen == 0) {
        return;
    }
    /* XOR each byte of data with corresponding byte of key (cycling through key) */
    for (i = 0; i < n; i++) {
        data[i] ^= key[i % keyLen];
    }
}

int encrypt_data(unsigned char *data, size_t n, const char *pin) {

    unsigned char key[256];  /* Buffer for derived key */
    size_t keyLen;
    
    /* Validate inputs */
    if (!data || !pin || n == 0) {
        return -1;  /* Error: invalid parameters */
    }
    
    /* Derive encryption key from PIN */
    keyLen = (n < 256) ? n : 256;
    if (derive_key(pin, key, keyLen) == 0) {
        return -1;  /* Error: key derivation failed */
    }
    
    /* Apply XOR cipher to encrypt the data */
    xor_cipher(data, n, key, keyLen);
    
    return 0;  /* Success! Yayyayay! */
}

int decrypt_data(unsigned char *data, size_t n, const char *pin) {

    unsigned char key[256];
    size_t keyLen;
    
    /* Validate inputs */
    if (!data || !pin || n == 0) {
        return -1;
    }
    
    /* Derive decryption key (same process as encryption) */
    keyLen = (n < 256) ? n : 256;
    if (derive_key(pin, key, keyLen) == 0) {
        return -1;
    }
    
    /* Apply XOR cipher to decrypt */
    xor_cipher(data, n, key, keyLen);
    
    return 0;
}


/* ========== File Integrity Implementation ========== */

unsigned long compute_file_hash(const unsigned char *data, size_t n) {

    unsigned long hash;
    size_t i;
    
    if (!data || n == 0) {
        return 0;
    }
    
    /* Start with a different seed than PIN hash to avoid collisions */
    hash = 2166136261UL;  /* FNV offset basis */
    
    for (i = 0; i < n; i++) {
        hash ^= (unsigned long)data[i];
        hash *= 16777619UL; /* FNV prime */
    }
    
    return hash;
}

int verify_file_integrity(const unsigned char *data, size_t n, unsigned long storedHash) {

    unsigned long computedHash;
    
    if (!data) {
        return 0;  /* Can't verify null data */
    }
    
    /* Compute hash of current data */
    computedHash = compute_file_hash(data, n);
    
    /* Compare with stored hash - return 1 if match, 0 if mismatch */
    return (computedHash == storedHash) ? 1 : 0;
}


/* ========== Utility Functions ========== */

int verify_pin(const char *pin, unsigned long storedHash) {
    /* Check if a PIN is correct by comparing hashes */
    unsigned long computedHash;
    
    if (!pin) {
        return 0;  /* Null PIN doesn't match anything */
    }
    
    /* Hash the provided PIN and compare with stored hash */
    computedHash = hash_pin(pin);
    
    /* Return 1 for match, 0 for no match */
    return (computedHash == storedHash) ? 1 : 0;
}
