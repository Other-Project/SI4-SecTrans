#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <sodium.h>

typedef struct {
    unsigned char private_key[crypto_box_SECRETKEYBYTES];
    unsigned char public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char nonce[crypto_box_NONCEBYTES];
} ENCRYPTION_TOOLS;

void generate_encryption_tools(ENCRYPTION_TOOLS *encryption_tools)
{
    crypto_box_keypair(encryption_tools->public_key, encryption_tools->private_key);
    randombytes_buf(encryption_tools->nonce, sizeof(encryption_tools->nonce));
}

#endif