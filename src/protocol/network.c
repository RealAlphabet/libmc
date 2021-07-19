#include <unistd.h>

#include "libmc.h"


///////////////////////////////////
//  UTIL
///////////////////////////////////


void packet_send(int fd, buffer_t *buf)
{
    size_t size     = buf->pos - 4;
    size_t extra    = buffer_size_varint(size);
    size_t start    = 4 - extra;

    // Write packet length.
    buf->pos = start;
    buffer_write_varint(buf, size);

    // Restore packet position.
    buf->pos = size + 4;
    size    += extra;

    // Send packet.
    write(fd, &buf->data[start], size);
}


///////////////////////////////////
//  PACKET
///////////////////////////////////


//  HANDSHAKE

void packet_send_handshake(int fd, pkt_handshake_t handshake)
{
    char buf[256];
    buffer_t buffer = { buf, 256, 4 };

    // Send handshake packet.
    buffer_write_varint(&buffer, 0x00);
    buffer_write_varint(&buffer, handshake.version);                        // Protocol ID
    buffer_write_string(&buffer, handshake.host, strlen(handshake.host));   // Server
    buffer_write_u16(&buffer, handshake.port);                              // Port
    buffer_write_u8(&buffer, handshake.state);                              // State
    packet_send(fd, &buffer);
}

//  LOGIN

void packet_send_login(int fd, const char *username)
{
    char buf[256];
    buffer_t buffer = { buf, 256, 4 };

    // Send login packet.
    buffer_write_varint(&buffer, 0x00);
    buffer_write_string(&buffer, username, strlen(username));
    packet_send(fd, &buffer);
}

void packet_send_encryption_response(int fd, crypto_context_t *context, const char *token, size_t len)
{
    char sharedOutput[128];
    char verifyOutput[128];
    char buf[512];
    buffer_t buffer = { buf, 512, 4 };

    // Encrypt shared key and token.
    crypto_rsa_encrypt(context, context->iv ,  16 , sharedOutput, 128);
    crypto_rsa_encrypt(context, token       , len , verifyOutput, 128);

    // Send encryption response packet.
    buffer_write_varint(&buffer, 0x01);
    buffer_write_array(&buffer, sharedOutput, 128);
    buffer_write_array(&buffer, verifyOutput, 128);
    packet_send(fd, &buffer);
}