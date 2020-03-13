#ifndef ABSTRACTVARS_H
#define ABSTRACTVARS_H

#include <cx_mem_abstracts/abstractvar.h>
#include <map>
#include <list>

// TODO: limits...

class AbstractVars
{
public:
    AbstractVars();
    virtual ~AbstractVars();

    /**
     * @brief set Set the variable (as a list of variables)
     * @param varName variable name.
     * @param vars variables values.
     */
    void set(const std::string & varName, AbstractVars * vars);

    void setFromString(const std::string & varName, AVarType varType, const std::string & str);
    /**
     * @brief set Set variable.
     *               The variable will be destroyed
     * @param varName variable name.
     * @param var variable value.
     */
    void set(const std::string & varName, AbstractVar * var);
    /**
     * @brief getAsString Get variable as string.
     * @param varName variable name.
     * @return string if found, or empty string if not.
     */
    std::string getAsString(const std::string & varName);
    /**
     * @brief rem remove variable.
     * @param varName variable name
     */
    void rem(const std::string & varName);
    /**
     * @brief get Get variable as abstract
     * @param varName variable name.
     * @return nullptr if not found,  AbstractVar if found.
     */
    AbstractVar * get(const std::string & varName);

    AbstractVars * getList(const std::string & varName);


    std::list<std::string> getVarKeys();
    std::list<std::string> getVarListKeys();

private:
    std::map<std::string, AbstractVar *> vars;
    std::map<std::string, AbstractVars *> varsList;

};

#endif // ABSTRACTVARS_H
