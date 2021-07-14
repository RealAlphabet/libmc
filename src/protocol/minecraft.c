#include <unistd.h>
#include <stdlib.h>

#include "http.h"
#include "minecraft.h"


///////////////////////////////////
//  PACKET
///////////////////////////////////


void send_packet(int fd, buffer_t *buf)
{
    size_t size     = buf->pos - 4;
    size_t extra    = buffer_size_varint(size);
    size_t start    = 4 - extra;

    // Write packet length.
    buf->pos = start;
    buffer_write_varint(buf, size);

    // Restore packet position.
    buf->pos = size + 4;
    size += extra;

    // Send packet.
    write(fd, &buf->data[start], size);
}


///////////////////////////////////
//  ENCRYPTION
///////////////////////////////////


void mojang_auth_login(mojang_session_t *session)
{
    char digest[42];
    char buf[1024];
    http_response_t response = { 0 };

    // Compute SHA1 digest.
    crypto_sha1(digest, (sha1_entry_t[4]){
        { session->id     , session->id_len   },
        { session->shared , 16                },
        { session->key    , session->key_len  },
        { 0 }
    });

    // Build JSON request body.
    sprintf(buf, "{\"accessToken\":\"%s\",\"selectedProfile\":\"%s\",\"serverId\":\"%s\"}", session->token, session->profile, digest);

    // Send join request to Mojang API.
    http_post("https://sessionserver.mojang.com/session/minecraft/join", buf, &response);

    if (response.status == 204) {
        printf("OK.\n");

    } else {
        printf("%s\n", response.buf);
        free(response.buf);
    }
}

void packet_send_encryption_response(int fd, crypto_context_t *context, const char *token, size_t len)
{
    char sharedOutput[128];
    char verifyOutput[128];
    char buf[288];
    buffer_t packet = { buf, 288, 4 };

    // Encrypt shared key and token.
    crypto_rsa_encrypt(context, context->iv ,  16 , sharedOutput, 128);
    crypto_rsa_encrypt(context, token       , len , verifyOutput, 128);

    // Send encryption response packet.
    buffer_write_varint(&packet, 0x01);
    buffer_write_array(&packet, sharedOutput, 128);
    buffer_write_array(&packet, verifyOutput, 128);
    send_packet(fd, &packet);
}
