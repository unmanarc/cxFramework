#include "abstractvars.h"

AbstractVars::AbstractVars()
{

}

AbstractVars::~AbstractVars()
{
    for (const auto & i : vars) delete i.second;
    for (const auto & i : varsList) delete i.second;
    vars.clear();
    varsList.clear();
}

void AbstractVars::set(const std::string &varName, AbstractVars *vars)
{
    rem(varName);
    varsList[varName] = vars;
}

void AbstractVars::setFromString(const std::string &varName, AVarType varType, const std::string &str)
{
    set(varName, AbstractVar::makeAbstractVar(varType,str));
}

void AbstractVars::set(const std::string &varName, AbstractVar *var)
{
    rem(varName);
    vars[varName] = var;
}

std::string AbstractVars::getAsString(const std::string &varName)
{
    if (vars.find(varName) == vars.end())
        return "";
    return vars[varName]->toString();
}

void AbstractVars::rem(const std::string &varName)
{
    if (vars.find(varName) != vars.end())
    {
        delete vars[varName];
        vars.erase(varName);
    }
    if (varsList.find(varName) != varsList.end())
    {
        delete varsList[varName];
        varsList.erase(varName);
    }
}

AbstractVar *AbstractVars::get(const std::string &varName)
{
    if (vars.find(varName) == vars.end()) return nullptr;
    return vars[varName];
}

AbstractVars *AbstractVars::getList(const std::string &varName)
{
    if (varsList.find(varName) == varsList.end()) return nullptr;
    return varsList[varName];
}

std::list<std::string> AbstractVars::getVarKeys()
{
    std::list<std::string> r;
    for ( const auto & i : vars ) r.push_back(i.first);
    return r;
}

std::list<std::string> AbstractVars::getVarListKeys()
{
    std::list<std::string> r;
    for ( const auto & i : varsList ) r.push_back(i.first);
    return r;
}
