#pragma once

#include "../include/crypto.h"
#include "../include/buffer.h"


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct connection_t         connection_t;
typedef struct packet_handler_t     packet_handler_t;

struct packet_handler_t
{
    void                (*on_read)(connection_t *connection, buffer_t *packet, packet_handler_t *next);
    void                (*on_write)(connection_t *connection, buffer_t *packet, packet_handler_t *next);
    packet_handler_t    *next;
};

struct connection_t
{
    int                 fd;
    int                 threshold;  // Compression threshold (-1 if disabled)
    crypto_context_t    *crypto;
    packet_handler_t    handler;
};

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
//  PACKET
///////////////////////////////////


void send_packet(int fd, buffer_t *buf);


///////////////////////////////////
//  ENCRYPTION
///////////////////////////////////


void mojang_auth_login(mojang_session_t *session);
void packet_send_encryption_response(int fd, crypto_context_t *context, const char *token, size_t len);
