#ifndef HTTP_CODE_RESPONSE_H
#define HTTP_CODE_RESPONSE_H

#include <cx_mem_streamparser/substreamparser.h>
#include "http_version.h"
#include "http_retcodes.h"

struct sResponseCode
{
    sResponseCode(const uint16_t &code,const char *responseMessage)
    {
        this->code = code;
        this->responseMessage = responseMessage;
    }
    uint16_t code;
    std::string responseMessage;
};

class HTTP_Status : public SubStreamParser
{
public:
    HTTP_Status();
    /**
     * @brief getHttpVersion - Get HTTP Version Object
     * @return Version Object
     */
    HTTP_Version * getHttpVersion();
    /**
     * @brief getResponseCode - Get HTTP Response Code (Ex. 404=Not found)
     * @return response code number
     */
    unsigned short getRetCode() const;
    /**
     * @brief setResponseCode - Set HTTP Response Code (Ex. 404=Not found)
     * @param value response code number
     */
    void setRetCodeValue(unsigned short value);
    /**
     * @brief setResponseCode2 Set response code and message from a fixed list.
     */
    void setRetCode(HttpRetCode code);
    /**
     * @brief getResponseMessage - Get HTTP Response Code Message (Ex. Not found)
     * @return response code message
     */
    std::string getResponseMessage() const;
    /**
     * @brief setResponseMessage - Set HTTP Response Code Message (Ex. Not found)
     * @param value response code message
     */
    void setResponseMessage(const std::string &value);

    bool stream(WRStatus & wrStat) override;
protected:
    ParseStatus parse() override;

private:
    HTTP_Version httpVersion;
    unsigned short responseCode;
    std::string responseMessage;

};

#endif // HTTP_CODE_RESPONSE_H
