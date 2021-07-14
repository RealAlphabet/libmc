#pragma once

#include "crypto.h"
#include "pipeline.h"


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct
{
    int                 fd;
    int                 threshold;  // Compression threshold (-1 if disabled)
    crypto_context_t    *crypto;
    pipeline_entry_t    *pipeline;
} connection_t;

typedef struct
{
    const char      *token;
    const char      *profile;
    const char      *shared;
    const char      *id;
    size_t          id_len;
    const char      *key;
    size_t          key_len;
} mojang_session_t;


///////////////////////////////////
//  HANDLER
///////////////////////////////////


typedef void (*decoder_t)(connection_t *connection, buffer_t *packet);

//  handler.c
void on_packet_decrypt(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_packet_decompress(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_login_packet_raw(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_packet_otoak(connection_t *connection, buffer_t *packet, packet_handler_t *next);

//  handler.c
void packet_on_disconnect(connection_t *connection, buffer_t *packet);
void packet_on_encryption_request(connection_t *connection, buffer_t *packet);
void packet_on_login_success(connection_t *connection, buffer_t *packet);
void packet_on_set_compression(connection_t *connection, buffer_t *packet);


///////////////////////////////////
//  PACKET
///////////////////////////////////


//  minecraft.c
void send_packet(int fd, buffer_t *buf);


///////////////////////////////////
//  ENCRYPTION
///////////////////////////////////


//  minecraft.c
void mojang_auth_login(mojang_session_t *session);
void packet_send_encryption_response(int fd, crypto_context_t *context, const char *token, size_t len);
