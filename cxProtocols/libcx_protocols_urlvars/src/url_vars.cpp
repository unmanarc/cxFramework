#include "url_vars.h"

#include <cx_mem_streamencoders/streamencoder_url.h>

#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace boost::algorithm;

URL_Vars::URL_Vars(StreamableObject *value) : StreamParser(value,false)
{
    initSubParser(&_urlVarParser);

    initialized = initProtocol();
    currentStat = URLV_STAT_WAITING_NAME;
    _urlVarParser.setVarType(true);

    setMaxVarNameSize(4096);
    setMaxVarContentSize(4096);

    currentParser = &_urlVarParser;
}

URL_Vars::~URL_Vars()
{
    for (auto & i : vars) delete i.second;
}

bool URL_Vars::isEmpty()
{
    return vars.empty();
}

bool URL_Vars::streamTo(StreamableObject *out, WRStatus &wrsStat)
{
    WRStatus cur;
    bool firstVar = true;
    for (auto & i : vars)
    {
        if (firstVar) firstVar=false;
        else
        {
            if (!(cur+=out->writeString("&", wrsStat)).succeed)
                return false;
        }

        BinaryContainer_Chunks varName;
        varName.append(i.first.c_str(), i.first.size());

        StreamEncoder_URL varNameEncoder(out);
        //bytesWritten+=varNameEncoder.getFinalBytesWritten();
        if (!(cur+=varName.streamTo(&varNameEncoder, wrsStat)).succeed)
        {
            out->writeEOF(false);
            return false;
        }

        if (((BinaryContainer_Chunks *)i.second)->size())
        {
            if (!(cur+=out->writeString("=",wrsStat)).succeed)
                return false;

            StreamEncoder_URL varNameEncoder2(out);
            //writtenBytes+=varNameEncoder2.getFinalBytesWritten();
            if (!((BinaryContainer_Chunks *)i.second)->streamTo(&varNameEncoder2,wrsStat))
            {
                out->writeEOF(false);
                return false;
            }
        }
    }
    out->writeEOF(true);
    return true;
}

uint32_t URL_Vars::varCount(const std::string &varName)
{
    uint32_t i=0;
    auto range = vars.equal_range(boost::to_upper_copy(varName));
    for (auto iterator = range.first; iterator != range.second;) i++;
    return i;
}

BinaryContainer_Base *URL_Vars::getValue(const std::string &varName)
{
    auto range = vars.equal_range(boost::to_upper_copy(varName));
    for (auto iterator = range.first; iterator != range.second;) return iterator->second;
    return nullptr;
}

std::list<BinaryContainer_Base *> URL_Vars::getValues(const std::string &varName)
{
    std::list<BinaryContainer_Base *> r;
    auto range = vars.equal_range(boost::to_upper_copy(varName));
    for (auto iterator = range.first; iterator != range.second;) r.push_back(iterator->second);
    return r;
}

std::set<std::string> URL_Vars::getKeysList()
{
    std::set<std::string> r;
    for ( const auto & i : vars ) r.insert(i.first);
    return r;
}

bool URL_Vars::initProtocol()
{
    return true;
}

void URL_Vars::endProtocol()
{
}

bool URL_Vars::changeToNextParser()
{
    switch(currentStat)
    {
    case URLV_STAT_WAITING_NAME:
    {
        currentVarName = _urlVarParser.flushRetrievedContentAsString();
        if (_urlVarParser.getDelimiterFound() == "&" || _urlVarParser.isStreamEnded())
        {
            // AMP / END:
            insertVar(currentVarName, _urlVarParser.flushRetrievedContentAsBC());
        }
        else
        {
            // EQUAL:
            currentStat = URLV_STAT_WAITING_CONTENT;
            _urlVarParser.setVarType(false);
            _urlVarParser.setMaxObjectSize(maxVarContentSize);
        }
    }break;
    case URLV_STAT_WAITING_CONTENT:
    {
        insertVar(currentVarName, _urlVarParser.flushRetrievedContentAsBC());
        currentStat = URLV_STAT_WAITING_NAME;
        _urlVarParser.setVarType(true);
        _urlVarParser.setMaxObjectSize(maxVarNameSize);
    }break;
    default:
        break;
    }

    return true;
}

void URL_Vars::insertVar(const std::string &varName, BinaryContainer_Chunks *data)
{
    //vars.insert(std::pair<std::string,BinaryContainer_Chunks*>(currentVarName, _urlVarParser.flushRetrievedContentAsBC()));
    if (!varName.empty())
        vars.insert(std::pair<std::string,BinaryContainer_Chunks*>(boost::to_upper_copy(varName), data));
    else
        delete data;
}


void URL_Vars::iSetMaxVarContentSize()
{
    if (currentStat == URLV_STAT_WAITING_CONTENT) _urlVarParser.setMaxObjectSize(maxVarContentSize);
}

void URL_Vars::iSetMaxVarNameSize()
{
    if (currentStat == URLV_STAT_WAITING_NAME) _urlVarParser.setMaxObjectSize(maxVarNameSize);
}
