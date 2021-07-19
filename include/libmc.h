#pragma once


///////////////////////////////////
//  MOJANG API
///////////////////////////////////


typedef struct
{
    const char *username;
    const char *token;
    const char *uuid;
} mojang_account_t;

typedef struct
{
    const char      *token;
    const char      *secret;
    const char      *serverid;
    const char      *pubkey;
    size_t          len_serverid;
    size_t          len_pubkey;
} mojang_server_session_t;

int mojang_auth_login(mojang_account_t *account, const char *email, const char *password);
int mojang_auth_join(mojang_account_t *account, mojang_server_session_t *session);
