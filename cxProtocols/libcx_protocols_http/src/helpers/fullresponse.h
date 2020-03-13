#ifndef FULLRESPONSE_H
#define FULLRESPONSE_H

#include "http_cookies_serverside.h"
#include "http_content.h"
#include "http_status.h"

struct sWebFullResponse
{
    // Proceced information:
    HTTP_Cookies_ServerSide * SET_COOKIES;

    // Original Values:
    MIME_Sub_Header * headers; //
    HTTP_Status * status; //
    HTTP_Content * contentData; //
};


#endif // FULLRESPONSE_H
