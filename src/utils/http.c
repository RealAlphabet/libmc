#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "http.h"


///////////////////////////////////
//  CALLBACK
///////////////////////////////////


size_t write_callback(char *data, size_t size, size_t nmemb, http_response_t *response)
{
    // Get real size.
    size *= nmemb;

    // Realloc buffer.
    response->buf = realloc(response->buf, (response->size + size + 1));

    // Copy chunk response.
    memcpy(&response->buf[response->size], data, size);

    // Update buffer.
    response->size                  += size;
    response->buf[response->size]   = 0;

    return (size);
}


///////////////////////////////////
//  REQUEST
///////////////////////////////////


void http_post(const char *url, char *data, http_response_t *response)
{
    CURL *curl;
    struct curl_slist *headers;

    // Init curl request.
    if ((curl = curl_easy_init()) == NULL)
        return;

    // Set request headers.
    headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Set request options.
    curl_easy_setopt(curl, CURLOPT_URL              , url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER       , headers);
    curl_easy_setopt(curl, CURLOPT_POST             , 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS       , data);

    // Set request write function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION    , write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA        , response);

    // Send request and get status code.
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,  &response->status);

    // Cleanup.
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
