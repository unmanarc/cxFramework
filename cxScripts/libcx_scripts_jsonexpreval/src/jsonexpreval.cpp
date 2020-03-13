#include "jsonexpreval.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;

JSONExprEval::JSONExprEval()
{
    negativeExpression = false;
    staticTextsOwner = false;
    staticTexts = nullptr;
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
    compile(expr);
}

JSONExprEval::JSONExprEval(const string &expr, std::vector<string> *staticTexts, bool negativeExpression)
{
    this->negativeExpression = negativeExpression;
    staticTextsOwner = false;
    this->staticTexts = staticTexts;
    compile(expr);
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
    int dsub;
    while ((dsub=detectSubExpr(expr))==0) {}
    if (dsub == -1)
    {
        lastError = "bad parenthesis balancing";
        return false;
    }

    // Separate AND/OR
    if (  expr.find(" and ") != std::string::npos &&  expr.find(" or ") != std::string::npos )
    {
        evalMode = EVAL_MODE_UNDEFINED;
        lastError = "Expression with both and/or and no precedence order";
        return false;
    }
    else
    {
        // split
        if (expr.find(" and ") != std::string::npos)
        {
            evalMode = EVAL_MODE_AND;
            boost::replace_all(expr," and " ,"\n");
        }
        else
        {
            evalMode = EVAL_MODE_OR;
            boost::replace_all(expr," or " ,"\n");
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

int JSONExprEval::detectSubExpr(string &expr)
{
    int level=0;
    bool inSubExpr = false;

    size_t firstByte=0;

    for (size_t i=0;i<expr.size();i++)
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
            if (level == 0) return -1;
            level--;
            if (level==0 && inSubExpr)
            {
                /////////////////////////////
                std::string subexpr = expr.substr(firstByte+1,i-firstByte-1);

                uint64_t pos = subExpressions.size();
                char _staticmsg[128];
                _staticmsg[127]=0;
                snprintf(_staticmsg,127,"_SUBEXPR_%lu",pos);

                if (firstByte>0 && expr.at(firstByte-1)=='!')
                {
                    subExpressions.push_back(new JSONExprEval(subexpr,staticTexts,true));
                    boost::replace_first(expr,"!(" + subexpr + ")" ,_staticmsg);
                }
                else
                {
                    subExpressions.push_back(new JSONExprEval(subexpr,staticTexts,false));
                    boost::replace_first(expr,"(" + subexpr + ")" ,_staticmsg);
                }

                return 0;
            }
        }
    }
    return 1;
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
