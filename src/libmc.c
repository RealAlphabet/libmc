#include <curl/curl.h>

#include "libmc.h"


///////////////////////////////////
//  INIT
///////////////////////////////////


void libmc_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
    crypto_setup();
}

void libmc_cleanup(void)
{
    crypto_destroy();
    curl_global_cleanup();
}


///////////////////////////////////
//  LIBMC
///////////////////////////////////


void libmc_close(libmc_t *mc)
{
    free(mc->fd);
}

void libmc_read(libmc_t *mc)
{
    
}


///////////////////////////////////
//  SYNCHRONIZED
///////////////////////////////////


int libmc_sync_login(libmc_t *mc)
{
    static char     buf[32768];
    buffer_t        buffer      = { buf, 32768, 0 };
    connection_t    connection  = { 0 };
    size_t          len;

    // Connect to minecraft server.
    if ((connection.fd = socket_connect(host, port)) == -1)
        return (-1);


// Handshake & login

    // Set network pipe handler and reset packet position.
    connection.pipeline = pipeline_create(PIPE_RAW, (packet_handler_t)handler_raw, NULL);
    buffer.pos          = 0;

    // Read stream and additional packet data.
    while ((len = read(connection.fd, &buf[buffer.pos], (32768 - buffer.pos))) > 0) {

        // Increment buffer size.
        buffer.size += len;

        // Decode and handle buffers.
        connection.pipeline->on_read(&connection, &buffer, connection.pipeline->next);
    }

    // Free crypto context.
    if (connection.crypto) {
        crypto_free(connection.crypto);
        free(connection.crypto);
    }

    // Free pipeline.
    pipeline_free(&connection.pipeline);

    // Close connection.
    close(connection.fd);

    return (0);
}

int libmc_sync_ping(libmc_t *mc)
{

}
