#ifndef GLOBALARGUMENTS_H
#define GLOBALARGUMENTS_H

#include <string>
#include <list>
#include <map>
#include <cx_mem_abstracts/abstractvar.h>
#include <cx_thr_mutex/lock_mutex_shared.h>

#include "programvalues.h"

struct sProgCMDOpts {
    sProgCMDOpts()
    {
        defaultValueVar = nullptr;
        val = 0;
        required = false;
        varType = ABSTRACT_BOOL;
    }

    std::string defaultValue;
    std::string description;

    AVarType varType;
    bool required;

    AbstractVar * defaultValueVar;
    std::list<AbstractVar *> parsedOption;


    //
    std::string name;
    char val;
};

class GlobalArguments : public ProgramValues
{
public:
    GlobalArguments();

    // Program Options
    /**
     * @brief addCommandLineOption Add command line option
     * @param optGroup Option Group Name (for help)
     * @param optChar Option short name (one character or '\0')
     * @param optName Option full name
     * @param description Option description (for help)
     * @param defaultValue default value (will be translated)
     * @param varType Abtract Variable type
     * @param required Required
     * @return true if added, otherwise false.
     */
    bool addCommandLineOption(const std::string & optGroup, char optChar, const std::string & optName, const std::string & description, const std::string & defaultValue, const AVarType &varType, bool required = false);
    /**
     * @brief getCommandLineOptionBooleanValue Get Command Line Boolean User introduced Value
     * @param optionName Option Name (Full name)
     * @return option value (true or false)
     */
    bool getCommandLineOptionBooleanValue( const std::string & optionName );
    /**
     * @brief getCommandLineOptionValue Get Command Line Option Value (As abstract)
     * @param optionName Option Name (Full name)
     * @return option value (as an abstract, will match with defined varType in addCommandLineOption)
     */
    AbstractVar * getCommandLineOptionValue(const std::string & optionName );
    /**
     * @brief getCommandLineOptionValues Get Command Line Option Values (As list of abstracts)
     * @param optionName Option Name (Full name)
     * @return option values list
     */
    std::list<AbstractVar *> getCommandLineOptionValues(const std::string & optionName );

    // Program variables.
    /**
     * @brief addStaticVariable Add static variable
     * @param name Variable Name
     * @param var Add Variable Pointer (abstract)
     * @return true if added, false if not
     */
    bool addStaticVariable(const std::string & name, AbstractVar * var);
    /**
     * @brief getStaticVariable Get static variable
     * @param name Variable Name
     * @return nullptr if not found or pointer to Abstract Variable Pointer (will keep the same introduced pointer)
     */
    AbstractVar * getStaticVariable(const std::string & name);


    // Print Help options
    /**
     * @brief printHelp Print help options
     */
    void printHelp();

    // Print the banner.
    /**
     * @brief printProgramHeader Print program banner
     */
    void printProgramHeader();

    // Wait forever functions:
    bool isInifiniteWaitAtEnd() const;
    void setInifiniteWaitAtEnd(bool value);

    /**
     * @brief getDefaultHelpOption Get Default option to be used for help (Eg. help for --help)
     * @return default help option
     */
    std::string getDefaultHelpOption() const;
    void setDefaultHelpOption(const std::string &value);

    /**
     * @brief getDefaultDaemonOption Get Default option to deamon compatible (Eg. daemon for --daemon)
     * @return default daemon option
     */
    std::string getDefaultDaemonOption() const;
    void setDefaultDaemonOption(const std::string &value);


// INTERNAL FUNCTIONS:
    bool parseCommandLineOptions(int argc, char *argv[]);

private:
    std::string sDefaultHelpOption, sDefaultDaemonOption;

    std::list<sProgCMDOpts *> getAllCMDOptions();
    uint32_t getMaxOptNameSize(std::list<sProgCMDOpts *> options);
    std::string getLine(const uint32_t &size);

    sProgCMDOpts * getProgramOption(char optChar);
    sProgCMDOpts * getProgramOption(const std::string & optName);

    bool inifiniteWaitAtEnd;

    std::map<std::string,std::list<sProgCMDOpts *>> cmdOptions; // group->list of command options
    // TODO: multimap
    std::map<std::string,AbstractVar *> variables; // variable name -> variable

    Mutex_RW mutex_vars;
};

#endif // GLOBALARGUMENTS_H
