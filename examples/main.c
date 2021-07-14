#include <libmc/libmc.h>


///////////////////////////////////
//  HANDLER
///////////////////////////////////


void on_packet_receive(libmc_t *mc, buffer_t *packet)
{
    int id = buffer_read_varint(packet);

    printf("RECEIVED PACKET ID : %d\n", id);
}


///////////////////////////////////
//  MAIN
///////////////////////////////////


int main(int argc, char **argv)
{
    libmc_t mc;
    libmc_session_t session = { "token", "uuid" };

    // Initialize libmc.
    libmc_init();

    {
        // Connect to a minecraft server.
        // mc don't need to bet filled with 0.
        libmc_connect(&mc, "mc.hypixel.net", 25565, 47 /* protocol version */);

        // Wait for libmc to handle authentification.
        while (libmc_handle_login(mc, session));

        // Authentification is done, we are in PLAY state.
        while (libmc_handle_play(mc, on_packet_receive));

        // Close socket and free all associated ressources.
        libmc_close(&mc);
    }

    // Cleanup libmc.
    libmc_cleanup();

    return (0);
}