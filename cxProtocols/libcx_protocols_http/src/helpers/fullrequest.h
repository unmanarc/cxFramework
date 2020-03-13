#ifndef FULLREQUEST_H
#define FULLREQUEST_H

#include <cx_mem_vars/vars.h>
#include "http_cookies_clientside.h"
#include "http_request.h"
#include "http_content.h"

struct sWebFullRequest
{
    // Host Information:
    const char * CLIENT_IP;
    std::string * VIRTUAL_HOST;
    uint16_t * VIRTUAL_PORT;

    // TODO: pass...
    // Authentication information
    std::string AUTH_USER,AUTH_PASS;

    // User Agent
    std::string USER_AGENT;

    // Proceced information:
    Vars *VARS_GET, *VARS_POST;
    HeaderOption * VARS_COOKIES;

    // Original Values:
    MIME_Sub_Header * clientHeaders; // User agent, ...
    HTTP_Request * clientRequest; // method type, and other options...
    HTTP_Content * clientContentData; // content data...
};


#endif // FULLREQUEST_H
