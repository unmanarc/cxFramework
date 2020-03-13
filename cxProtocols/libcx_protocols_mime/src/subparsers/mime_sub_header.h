#ifndef MIME_SUB_HEADER_H
#define MIME_SUB_HEADER_H

#include <cx_mem_streamparser/substreamparser.h>

#include <string>
#include <map>
#include <list>

/*
 * TODO: Security: check if other servers can handle the MIME properly...
 */

// ??
/**
 * @brief The HeaderOption struct
 */
class HeaderOption
{
public:
    HeaderOption()
    {
        maxHeaderOptSize=8*KB_MULT;
        curHeaderOptSize=0;
        maxSubOptionsCount=16;
    }

    std::string getSubVar(const std::string & subVarName)
    {
        if (subVar.find(subVarName) == subVar.end()) return "";
        return subVar[subVarName];
    }

    std::string getString();

    void addSubVar(const std::string & varName, const std::string & varValue);

    std::string getUpperName() const;

    std::string getOrigName() const;
    void setOrigName(const std::string &value);

    std::string getValue() const;
    void setValue(const std::string &value);

    std::string getOrigValue() const;
    void setOrigValue(const std::string &value);

    uint64_t getMaxSubOptions() const;
    void setMaxSubOptions(const uint64_t &value);

private:
    bool isPermited7bitCharset(const std::string & varX);

    uint64_t maxSubOptionsCount;
    uint64_t maxHeaderOptSize;
    uint64_t curHeaderOptSize;

    std::string origName;
    std::string origValue;
    std::string value;
    std::map<std::string,std::string> subVar;
};

class MIME_Sub_Header : public SubStreamParser
{
public:
    MIME_Sub_Header();
    ~MIME_Sub_Header() override;

    bool stream(WRStatus &wrStat) override;

    /**
     * @brief exist exist option.
     * @param optionName
     * @return
     */
    bool exist(const std::string &optionName) const;
    /**
     * @brief remove Remove option from MIME Structure.
     * @param optionName option Name.
     */
    void remove(const std::string &optionName);
    /**
     * @brief replace Remove the option (if exist), and add you own.
     * @param optionName option Name.
     * @param optionValue option value.
     */
    void replace(const std::string &optionName, const std::string &optionValue);
    /**
     * @brief add Add Header Option.
     * @param optionName option Name.
     * @param optionValue option value.
     * @param state 1: continue with the last option. 0: new option.
     */
    void add(const std::string &optionName, const std::string &optionValue, int state = 0);
    /**
     * @brief getOptionsByName Get option list by name (multiple options with the same name (eg. set-cookie)
     * @param varName value name.
     * @return list of HeaderOption's
     */
    std::list<HeaderOption *> getOptionsByName(const std::string & varName) const;
    /**
     * @brief getOptionByName Get the first value
     * @param varName
     * @return nullptr if not exist.
     */
    HeaderOption * getOptionByName(const std::string & varName) const;
    /**
     * @brief getOptionRawStringByName Get Option STD String By Name (raw, as came from the input)
     * @param varName variable name.
     * @return "" if nothing found, or the original option value.
     */
    std::string getOptionRawStringByName(const std::string & varName) const;
    /**
     * @brief getOptionValueStringByName
     * @param varName
     * @return
     */
    std::string getOptionValueStringByName(const std::string & varName) const;

    /**
     * @brief getOptionAsUINT64
     * @param varName
     * @param optExist
     * @return
     */
    uint64_t getOptionAsUINT64(const std::string & varName, uint16_t base = 10, bool * optExist = nullptr) const;
    //////////////////////////////////////////////////

    void addHeaderOption(HeaderOption *opt);

    //////////////////////////////////////////////////
    // Security:
    void setMaxOptionSize(const size_t &value);
    void setMaxOptions(const size_t &value);

    size_t getMaxSubOptionCount() const;
    void setMaxSubOptionCount(const size_t &value);

    size_t getMaxSubOptionSize() const;
    void setMaxSubOptionSize(const size_t &value);

protected:
    ParseStatus parse() override;

private:
    void parseSubValues(HeaderOption *opt, const std::string & strName, int state=0);

    HeaderOption * lastOpt;
    void parseOptionValue(std::string optionValue);

    std::multimap<std::string,HeaderOption *> headers;
    size_t maxOptions;
    size_t maxSubOptionCount, maxSubOptionSize;

};

#endif // MIME_SUB_HEADER_H
