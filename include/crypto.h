#pragma once

#include <mbedtls/aes.h>
#include <mbedtls/pk.h>


///////////////////////////////////
//  STRUCTURES
///////////////////////////////////


typedef struct
{
    mbedtls_pk_context          context_pk;
    mbedtls_aes_context         context_aes;
    char                        iv[16];
} crypto_context_t;

typedef struct
{
    const char  *value;
    size_t      len;
} sha1_entry_t;


///////////////////////////////////
//  ENCRYPT
///////////////////////////////////


int crypto_rsa_encrypt(crypto_context_t *context, const char *input, size_t ilen, char *output, size_t olen);
int crypto_aes_encrypt(crypto_context_t *context, const char *input, char *output, size_t len);
int crypto_aes_decrypt(crypto_context_t *context, const char *input, char *output, size_t len);
void crypto_aes_restart(crypto_context_t *context);


///////////////////////////////////
//  HASH
///////////////////////////////////


void crypto_sha1(char dst[42], sha1_entry_t *entries);


///////////////////////////////////
//  CRYPTO
///////////////////////////////////


//  Instance
void crypto_setup(void);
void crypto_destroy(void);

//  Context
void crypto_init(crypto_context_t *context, const char *pub_key, size_t len);
void crypto_free(crypto_context_t *context);
