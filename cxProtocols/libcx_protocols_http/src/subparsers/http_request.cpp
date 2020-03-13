#include "http_request.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

HTTP_Request::HTTP_Request()
{
    requestMethod = "GET"; // Default Method.

    setParseMode(PARSE_MODE_DELIMITER);
    setParseDelimiter("\r\n");
    setSecurityMaxURLSize(128*KB_MULT); // 128K
}

bool HTTP_Request::stream(WRStatus & wrStat)
{
    WRStatus cur;
    // Act as a client. Send data from here.
    if (!(cur+=upStream->writeString(requestMethod + " " + requestURI, wrStat )).succeed) return false;
    if (!getVars.isEmpty())
    {
        if (!(cur+=upStream->writeString("?",wrStat)).succeed) return false;
        if (!getVars.streamTo(upStream,wrStat)) return false;
    }
    if (!(cur+=upStream->writeString(" " + httpVersion.getHTTPVersionString() + string("\r\n"), wrStat )).succeed) return false;
    return true;
}

ParseStatus HTTP_Request::parse()
{
    std::string clientRequest = getParsedData()->toString();

    vector<string> requestParts;
    split(requestParts,clientRequest,is_any_of("\t "),token_compress_on);

    // We need almost 2 parameters.
    if (requestParts.size()<2) return PARSE_STAT_ERROR;

    requestMethod = boost::to_upper_copy(requestParts[0]);
    requestURI = requestParts[1];
    httpVersion.parseVersion(requestParts.size()>2?requestParts[2]:"HTTP/1.0");

    parseURI();

    return PARSE_STAT_GOTO_NEXT_SUBPARSER;
}

void HTTP_Request::parseURI()
{
    size_t found=requestURI.find("?");

    if (found!=std::string::npos)
    {
        // We have parameters..
        requestURIParameters = requestURI.c_str()+found+1;
        requestURI.resize(found);
        parseGETParameters();
    }
    else
    {
        // No parameters.
    }
}

void HTTP_Request::parseGETParameters()
{
    WRStatus x;
    BinaryContainer_Chunks bc;
    bc.append(requestURIParameters.c_str(),requestURIParameters.size());
    bc.streamTo(&getVars,x);
}

std::string HTTP_Request::getRequestURIParameters() const
{
    return requestURIParameters;
}

HTTP_Version * HTTP_Request::getHTTPVersion()
{
    return &httpVersion;
}

Vars *HTTP_Request::getVarsPTR()
{
    return &getVars;
}

std::string HTTP_Request::getRequestMethod() const
{
    return requestMethod;
}

void HTTP_Request::setRequestMethod(const std::string &value)
{
    requestMethod = value;
}

std::string HTTP_Request::getURI() const
{
    return requestURI;
}

void HTTP_Request::setRequestURI(const std::string &value)
{
    requestURI = value;
}

void HTTP_Request::setSecurityMaxURLSize(size_t value)
{
    setParseDataTargetSize(value);
}

