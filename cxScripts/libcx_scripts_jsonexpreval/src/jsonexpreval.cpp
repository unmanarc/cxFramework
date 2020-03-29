#include "jsonexpreval.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <clocale>

using namespace std;

JSONExprEval::JSONExprEval()
{
    negativeExpression = false;
    staticTextsOwner = false;
    staticTexts = nullptr;
    isCompiled = false;
    evalMode = EVAL_MODE_UNDEFINED;
}

JSONExprEval::~JSONExprEval()
{
    for (const auto&i: subExpressions)
        delete i;
    for (const auto&i: atomExpressions)
    {
        if (i.first) delete i.first;
    }
    if (staticTextsOwner) delete staticTexts;
}

JSONExprEval::JSONExprEval(const std::string &expr)
{
    negativeExpression = false;
    staticTextsOwner = false;
    staticTexts = nullptr; // create on compile.
    isCompiled = compile(expr);
}

JSONExprEval::JSONExprEval(const string &expr, std::vector<string> *staticTexts, bool negativeExpression)
{
    this->negativeExpression = negativeExpression;
    staticTextsOwner = false;
    this->staticTexts = staticTexts;
    isCompiled = compile(expr);
}

bool JSONExprEval::compile(std::string expr)
{
    if (!staticTexts)
    {
        staticTextsOwner = true;
        staticTexts = new std::vector<std::string>;
    }

    boost::trim(expr);
    expression = expr;

    boost::match_flag_type flags = boost::match_default;

    // PRECOMPILE _STATIC_TEXT
    boost::regex exStaticText("\"(?<STATIC_TEXT>[^\"]*)\"");
    boost::match_results<string::const_iterator> whatStaticText;
    for (string::const_iterator start = expr.begin(), end =  expr.end();
         boost::regex_search(start, end, whatStaticText, exStaticText, flags);
         start = expr.begin(), end =  expr.end())
    {
        uint64_t pos = staticTexts->size();
        char _staticmsg[128];
        _staticmsg[127]=0;
        snprintf(_staticmsg,127,"_STATIC_%lu",pos);
        staticTexts->push_back(string(whatStaticText[1].first, whatStaticText[1].second));
        boost::replace_all(expr,"\"" + string(whatStaticText[1].first, whatStaticText[1].second) + "\"" ,_staticmsg);
    }

    if (expr.find('\"') != std::string::npos)
    {
        // Error, bad
        lastError = "bad quoting in text";
        return false;
    }

    if (expr.find('\n') != std::string::npos)
    {
        lastError = "Mutiline expression not supported";
        return false;
    }

    if (expr.find("_SUBEXPR_") != std::string::npos)
    {
        lastError = "Invalid keyword _SUBEXPR_";
        return false;
    }

    // Compress double spaces...
    while ( expr.find("  ") != std::string::npos)
    {
        boost::replace_all(expr,"  ", " ");
    }

    // detect/compile sub expressions

    size_t sePos = 0;
    while ((sePos=detectSubExpr(expr,sePos))!=expr.size() && sePos!=(expr.size()+1))
    {
    }

    if (sePos == (expr.size()+1))
    {
        lastError = "bad parenthesis balancing";
        return false;
    }

    // Separate AND/OR
    if (  expr.find(" && ") != std::string::npos &&  expr.find(" || ") != std::string::npos )
    {
        evalMode = EVAL_MODE_UNDEFINED;
        lastError = "Expression with both and/or and no precedence order";
        return false;
    }
    else
    {
        // split
        if (expr.find(" && ") != std::string::npos)
        {
            evalMode = EVAL_MODE_AND;
            boost::replace_all(expr," && " ,"\n");
        }
        else
        {
            evalMode = EVAL_MODE_OR;
            boost::replace_all(expr," || " ,"\n");
        }

        if ( expr.find(" ") != std::string::npos )
        {
            lastError = "Invalid Operator (only and/or is admitted)";
            return false;
        }

        std::vector<std::string> vAtomicExpressions;
        boost::split(vAtomicExpressions,expr,boost::is_any_of("\n"));
        for ( const auto & atomicExpr : vAtomicExpressions)
        {
            if ( boost::starts_with(atomicExpr, "_SUBEXPR_") )
            {
                size_t subexpr_pos = stoul(atomicExpr.substr(9));
                if (subexpr_pos>=subExpressions.size())
                {
                    lastError = "Invalid Sub Expression #";
                    return false;
                }
                atomExpressions.push_back(std::make_pair(nullptr,subexpr_pos));
            }
            else
            {
                AtomicExpression * atomExpression = new AtomicExpression(staticTexts);
                if (!atomExpression->compile(atomicExpr))
                {
                    lastError = "Invalid Atomic Expression";
                    evalMode = EVAL_MODE_UNDEFINED;
                    delete atomExpression;
                    return false;
                }
                atomExpressions.push_back(std::make_pair(atomExpression,0));
            }
        }

    }

    lastError = "";
    return true;
}

bool JSONExprEval::evaluate(const Json::Value &values)
{
    switch (evalMode)
    {
    case EVAL_MODE_AND:
    {
        for (const auto & i : atomExpressions)
        {
            if (i.first)
            {
                if (!i.first->evaluate(values))
                    return calcNegative(false); // Short circuit.
            }
            else
            {
                if ( !subExpressions[i.second]->evaluate(values) )
                    return calcNegative(false); // Short circuit.
            }
        }
        return calcNegative(true);
    }break;
    case EVAL_MODE_OR:
    {
        for (const auto & i : atomExpressions)
        {
            if (i.first)
            {
                if (i.first->evaluate(values))
                    return calcNegative(true); // Short circuit.
            }
            else
            {
                if ( subExpressions[i.second]->evaluate(values) )
                    return calcNegative(true); // Short circuit.
            }
        }
        return calcNegative(false);
    }break;
    default:
        return false;
    }
}

size_t JSONExprEval::detectSubExpr(string &expr, size_t start)
{
    int level=0;
    bool inSubExpr = false;

    size_t firstByte=0;

    for (size_t i=start;i<expr.size();i++)
    {
        if (expr.at(i) == '(')
        {
            if (level == 0)
            {
                inSubExpr=true;
                firstByte = i;
            }
            level++;
        }
        else if (expr.at(i) == ')')
        {
            if (level == 0) return expr.size()+1;
            level--;
            if (level==0 && inSubExpr)
            {
                /////////////////////////////
                std::string subexpr = expr.substr(firstByte+1,i-firstByte-1);

                uint64_t pos = subExpressions.size();
                char _staticmsg[128];
                _staticmsg[127]=0;
                snprintf(_staticmsg,127,"_SUBEXPR_%lu",pos);

                if (firstByte>0 && isalnum(expr.at(firstByte-1)))
                {
                    // FUNCTION, do not replace, next evaluation should be done after it.
                    return i+1;
                }
                else if (firstByte>0 && expr.at(firstByte-1)=='!')
                {
                    subExpressions.push_back(new JSONExprEval(subexpr,staticTexts,true));
                    boost::replace_first(expr,"!(" + subexpr + ")" ,_staticmsg);
                    return 0;
                }
                else
                {
                    subExpressions.push_back(new JSONExprEval(subexpr,staticTexts,false));
                    boost::replace_first(expr,"(" + subexpr + ")" ,_staticmsg);
                    return 0;
                }
            }
        }
    }
    return expr.size();
}

bool JSONExprEval::getIsCompiled() const
{
    return isCompiled;
}

std::string JSONExprEval::getLastCompilerError() const
{
    return lastError;
}

bool JSONExprEval::calcNegative(bool r)
{
    if (negativeExpression) return !r;
    return r;
}
