#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/sha1.h>

#include "crypto.h"


///////////////////////////////////
//  GLOBAL
///////////////////////////////////


static mbedtls_ctr_drbg_context     ctr_drbg;
static mbedtls_entropy_context      entropy;


///////////////////////////////////
//  ENCRYPT
///////////////////////////////////


int crypto_rsa_encrypt(crypto_context_t *context, const char *input, size_t ilen, char *output, size_t olen)
{
    return (mbedtls_pk_encrypt(
        &context->context_pk,
        input,
        ilen,
        output,
        &(size_t){ 0 },
        olen,
        mbedtls_ctr_drbg_random,
        &ctr_drbg
    ));
}

int crypto_aes_encrypt(crypto_context_t *context, const char *input, char *output, size_t len)
{
    return (mbedtls_aes_crypt_cfb8(
        &context->context_aes,
        MBEDTLS_AES_ENCRYPT,
        len,
        context->iv,
        input,
        output
    ));
}

int crypto_aes_decrypt(crypto_context_t *context, const char *input, char *output, size_t len)
{
    return (mbedtls_aes_crypt_cfb8(
        &context->context_aes,
        MBEDTLS_AES_DECRYPT,
        len,
        context->iv,
        input,
        output
    ));
}


///////////////////////////////////
//  HASH
///////////////////////////////////


void crypto_sha1_hex(const char src[20], char dst[42])
{
    char format[]   = "0123456789abcdef";
    int i           = 0;

    // Checks if the digest should be negative.
    if (src[0] < 0) {
        dst[i++] = '-';

        // Convert dst to two's complement.
        *(int64_t*)(src +  0)    = *(int64_t*)(src +  0) * -1 - 1;
        *(int64_t*)(src +  8)    = *(int64_t*)(src +  8) * -1 - 1;
        *(int32_t*)(src + 16)    = *(int32_t*)(src + 16) * -1 - 1 + 0x1000000;
    }

    // Encode dst to hex.
    if (src[0] > 0xF) {
        dst[i++] = format[(src[0] >> 4) & 0xF];
    }

    dst[i++] = format[(src[0] >> 0) & 0xF];

    for (int j = 1; j < 20; j++) {
        dst[i++] = format[(src[j] >> 4) & 0xF];
        dst[i++] = format[(src[j] >> 0) & 0xF];
    }

    dst[i] = 0;
}

void crypto_sha1(char dst[42], sha1_entry_t *entries)
{
    char output[20];
    mbedtls_sha1_context context;

    // Init context.
    mbedtls_sha1_init(&context);
    mbedtls_sha1_starts(&context);

    // Update with all arguments.
    for (int i = 0; entries[i].value; i++) {
        mbedtls_sha1_update(&context, entries[i].value, entries[i].len);
    }

    // Finish and write output.
    mbedtls_sha1_finish(&context, output);
    mbedtls_sha1_free(&context);

    // Encode digest to hex.
    crypto_sha1_hex(output, dst);
}


///////////////////////////////////
//  CRYPTO
///////////////////////////////////


//  Instance

void crypto_setup(void)
{
    // Initialize entropy and drbg.
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // Seed the secure random number generator.
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, "preau_pepo_otoak3r", 18);
}

void crypto_destroy(void)
{
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
}

//  Context

void crypto_init(crypto_context_t *context, const char *key, size_t len)
{
    // Initialize RSA.
    mbedtls_pk_init(&context->context_pk);
    mbedtls_pk_parse_public_key(&context->context_pk, key, len);

    // Generate random 16 bytes IV.
    mbedtls_ctr_drbg_random(&ctr_drbg, context->iv, 16);

    // Initialize AES.
    mbedtls_aes_init(&context->context_aes);
    mbedtls_aes_setkey_enc(&context->context_aes, context->iv, 128);
}

void crypto_free(crypto_context_t *context)
{
    mbedtls_pk_free(&context->context_pk);
    mbedtls_aes_free(&context->context_aes);
}
