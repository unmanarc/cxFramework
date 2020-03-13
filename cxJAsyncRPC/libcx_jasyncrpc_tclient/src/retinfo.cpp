#include "retinfo.h"

void syncHandler(void *obj, int retCode, const Json::Value &payload, const Json::Value &extraInfo)
{
    sRetInfo * retInfo = (sRetInfo *)(obj);
    retInfo->payload = payload;
    retInfo->extraInfo = extraInfo;
    retInfo->retCode = retCode;
    retInfo->setWritten();
}
