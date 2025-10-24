/**
 * crypto.h - Cryptography Module Header
 * Handles encryption/decryption, key derivation, and PRNG for the locker system
 * Author: Edward Doan
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* PRNG Functions */
void prng_seed(unsigned long seed);
unsigned long prng_next(void);
unsigned char prng_byte(void);


/* Key Derivation Function */
size_t derive_key(const char *pin, unsigned char *out, size_t maxLen);
unsigned long hash_pin(const char *pin);

/* Encryption/Decryption Functions */
void xor_cipher(unsigned char *data, size_t n, const unsigned char *key, size_t keyLen);
int encrypt_data(unsigned char *data, size_t n, const char *pin);
int decrypt_data(unsigned char *data, size_t n, const char *pin);


/* File Integrity Functions */
unsigned long compute_file_hash(const unsigned char *data, size_t n);
int verify_file_integrity(const unsigned char *data, size_t n, unsigned long storedHash);


/* Pin Verification Functions */
int verify_pin(const char *pin, unsigned long storedHash);


#endif /* CRYPTO_H */
