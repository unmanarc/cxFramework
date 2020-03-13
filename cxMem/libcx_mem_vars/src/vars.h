#ifndef VARSCONTAINER_H
#define VARSCONTAINER_H

#include <cx_mem_containers/binarycontainer_base.h>
#include <string>
#include <list>
#include <set>

class Vars
{
public:
    Vars();
    virtual ~Vars();

    ///////////////////////////////////////
    // String conversion.
    std::string getStringValue(const std::string & varName);
    std::list<std::string> getStringValues(const std::string & varName);

    ///////////////////////////////////////
    // Var Existence.
    bool exist(const std::string & varName);

    ///////////////////////////////////////
    // Virtuals...
    virtual uint32_t varCount(const std::string & varName) = 0;
    virtual BinaryContainer_Base * getValue(const std::string & varName) = 0;
    virtual std::list<BinaryContainer_Base *> getValues(const std::string & varName) = 0;
    virtual std::set<std::string> getKeysList()=0;
    virtual bool isEmpty() = 0;

    ///////////////////////////////////////
    // Security options:
    uint32_t getMaxVarNameSize() const;
    void setMaxVarNameSize(const uint32_t &value);

    uint64_t getMaxVarContentSize() const;
    void setMaxVarContentSize(const uint64_t &value);

protected:
    virtual void iSetMaxVarContentSize();
    virtual void iSetMaxVarNameSize();

    // Security options.
    uint32_t maxVarNameSize;
    uint64_t maxVarContentSize;
};

#endif // VARSCONTAINER_H
