
///////////////////////////////////
//  LOGIN
///////////////////////////////////


void packet_on_disconnect(connection_t *connection, buffer_t *packet)
{
    fprintf(stderr, "DISCONNECTING.\n");
    exit(0);
}

void packet_on_encryption_request(connection_t *connection, buffer_t *packet)
{
    char server_id[20];
    char pub_key[512];
    char verify_token[16];

    // Read packet.
    buffer_read_string(packet , server_id    , 20);
    buffer_read_array(packet  , pub_key      , 512);
    buffer_read_array(packet  , verify_token , 16);

    // Init crypto.
    connection->crypto = malloc(sizeof(crypto_context_t));
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
    connection->handler.on_read         = on_packet_decrypt;
    connection->handler.next            = calloc(1, sizeof(packet_handler_t));
    connection->handler.next->on_read   = on_packet_raw;
}

void packet_on_login_success(connection_t *connection, buffer_t *packet)
{
    char uuid[64];
    char username[32];

    // Read packet.
    buffer_read_string(packet, uuid     , 64);
    buffer_read_string(packet, username , 32);

    // Set packet handler.
    connection->handler.next->next->on_read = on_packet_test;

    // Write a log message.
    printf("\n");
    printf("Connection successful !\n");
    printf("\tUUID:\t\t%s\n", uuid);
    printf("\tUsername:\t%s\n", username);
    printf("\n");
}

void packet_on_set_compression(connection_t *connection, buffer_t *packet)
{
    packet_handler_t *save = connection->handler.next;

    // Read packet.
    connection->threshold = buffer_read_varint(packet);

    // Save current handler and create a new.
    if (connection->threshold >= 0) {
        connection->handler.next            = calloc(1, sizeof(packet_handler_t));
        connection->handler.next->on_read   = on_packet_decompress;
        connection->handler.next->next      = save;

        // test
        connection->handler.next->next->on_read = on_packet_otoak;
    }
}
