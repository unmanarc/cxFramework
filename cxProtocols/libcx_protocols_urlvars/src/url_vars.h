#ifndef HTTPURLFORMATTEDVARS_H
#define HTTPURLFORMATTEDVARS_H

#include <cx_mem_streamparser/streamparser.h>
#include <cx_mem_vars/vars.h>
#include <map>

#include "urlvarcontent_subparser.h"

enum URLVStat
{
    URLV_STAT_WAITING_NAME,
    URLV_STAT_WAITING_CONTENT
};

class URL_Vars : public Vars, public StreamParser
{
public:
    URL_Vars(StreamableObject *value = nullptr);
    ~URL_Vars() override;

    /////////////////////////////////////////////////////
    // Stream Parsing:
    bool streamTo(StreamableObject * out, WRStatus & wrsStat) override;

    /////////////////////////////////////////////////////
    // Variables Container:
    uint32_t varCount(const std::string & varName) override;
    BinaryContainer_Base * getValue(const std::string & varName) override;
    std::list<BinaryContainer_Base *> getValues(const std::string & varName) override;
    std::set<std::string> getKeysList() override;
    bool isEmpty() override;

protected:
    void iSetMaxVarContentSize() override;
    void iSetMaxVarNameSize() override;

    bool initProtocol() override;
    void endProtocol() override;
    bool changeToNextParser() override;
private:
    void insertVar(const std::string & varName, BinaryContainer_Chunks * data);

    URLVStat currentStat;

    std::string currentVarName;
    std::multimap<std::string, BinaryContainer_Chunks *> vars;

    URLVarContent_SubParser _urlVarParser;
};

#endif // HTTPURLFORMATTEDVARS_H
