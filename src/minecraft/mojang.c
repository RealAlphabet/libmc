#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "libmc.h"


///////////////////////////////////
//  MOJANG API
///////////////////////////////////


int mojang_auth_login(mojang_account_t *account, const char *email, const char *password)
{
    char buf[512];
    http_response_t response = { 0 };

    // Build JSON request body.
    snprintf(buf, 512, "{\"agent\":{\"name\":\"Minecraft\",\"version\":1},\"username\":\"%s\",\"password\":\"%s\"}", email, password);

    // Send auth request to Mojang API.
    http_post("https://authserver.mojang.com/authenticate", buf, &response);

    // Get access token, UUID and username.
    if (response.status == 200) {
        printf("OK.\n");

    }

    // Free response buffer.
    if (response.buf)
        free(response.buf);
}

int mojang_auth_join(mojang_account_t *account, mojang_server_session_t *session)
{
    char digest[42];
    char buf[512];
    http_response_t response = { 0 };

    // Compute SHA1 digest.
    crypto_sha1(digest, (sha1_entry_t[4]){
        { session->id     , session->id_len   },
        { session->shared , 16                },
        { session->key    , session->key_len  },
        { 0 }
    });

    // Build JSON request body.
    snprintf(buf, 512, "{\"accessToken\":\"%s\",\"selectedProfile\":\"%s\",\"serverId\":\"%s\"}", session->token, session->profile, digest);

    // Send join request to Mojang API.
    http_post("https://sessionserver.mojang.com/session/minecraft/join", buf, &response);

    if (response.status == 204) {
        printf("OK.\n");

    } else {
        printf("%s\n", response.buf);
        free(response.buf);
    }
}
