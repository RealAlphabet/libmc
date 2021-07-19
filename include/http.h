#pragma once


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct {
    long        status;
    char        *buf;
    size_t      size;
} http_response_t;


///////////////////////////////////
//  REQUEST
///////////////////////////////////


void http_post(const char *url, char *data, http_response_t *response);
