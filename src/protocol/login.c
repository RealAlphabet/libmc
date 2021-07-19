#include <stdlib.h>
#include <string.h>

#include "minecraft.h"


///////////////////////////////////
//  LOGIN
///////////////////////////////////


void on_packet_disconnect(connection_t *connection, buffer_t *packet)
{
    char message[2048];

    // Read chat message.
    buffer_read_string(packet, message, 2048);

    // Print disconnect message.
    printf("DISCONNECTING.\n");
    printf("%s\n", message);
    exit(0);
}

void on_packet_encryption_request(connection_t *connection, buffer_t *packet)
{
    char server_id[20];
    char pub_key[512];
    char verify_token[16];

    // Read packet.
    buffer_read_string(packet , server_id    , 20);
    buffer_read_array(packet  , pub_key      , 512);
    buffer_read_array(packet  , verify_token , 16);

    // Allocate crypto.
    connection->crypto = malloc(sizeof(crypto_context_t));

    // Init crypto.
    crypto_init(connection->crypto, pub_key, 162);

    // Login to Mojang.
    mojang_auth_login(&(mojang_session_t){
        __MC_TOKEN,
        __MC_UUID,
        connection->crypto->iv,
        server_id,
        strlen(server_id),
        pub_key,
        162
    });

    // Send encryption response.
    packet_send_encryption_response(connection->fd, connection->crypto, verify_token, 4);

    // Set network handler.
    pipeline_add_before(&connection->pipeline, PIPE_RAW, pipeline_create(PIPE_DECRYPT, (packet_handler_t)handler_decrypt, NULL));
}

void on_packet_login_success(connection_t *connection, buffer_t *packet)
{
    char uuid[64];
    char username[32];

    // Read packet.
    buffer_read_string(packet, uuid     , 64);
    buffer_read_string(packet, username , 32);

    // Write a log message.
    printf("\n");
    printf("Connection successful !\n");
    printf("\tUUID:\t\t%s\n", uuid);
    printf("\tUsername:\t%s\n", username);
    printf("\n");
}

void on_packet_set_compression(connection_t *connection, buffer_t *packet)
{
    // Read packet.
    connection->threshold = buffer_read_varint(packet);

    // Add the decompress handler to the pipeline.
    if (connection->threshold >= 0)
        pipeline_add_before(&connection->pipeline, PIPE_RAW, pipeline_create(PIPE_DECOMPRESS, (packet_handler_t)handler_decompress, NULL));
}
