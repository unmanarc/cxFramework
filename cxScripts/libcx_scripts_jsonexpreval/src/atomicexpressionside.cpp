#include "atomicexpressionside.h"
#include <boost/algorithm/string.hpp>
#include <json/value.h>

#include <iostream>

using namespace std;

AtomicExpressionSide::AtomicExpressionSide(vector<string> *staticTexts)
{
    this->staticTexts = staticTexts;
    mode=EXPR_MODE_UNDEFINED;
    regexp = nullptr;
}

AtomicExpressionSide::~AtomicExpressionSide()
{
    if (regexp) delete regexp;
}

bool AtomicExpressionSide::calcMode()
{
    if (expr.empty()) mode=EXPR_MODE_NULL;
    else if ( expr.at(0)=='$') mode=EXPR_MODE_JSONPATH;
    else if ( expr.find_first_not_of("0123456789") == string::npos ) mode=EXPR_MODE_NUMERIC;
    else if ( boost::starts_with(expr,"_STATIC_") && staticTexts->size() > stoul(expr.substr(8)))
    {
        mode=EXPR_MODE_STATIC_STRING;
        staticIndex = stoul(expr.substr(8));
    }
    else
    {
        mode=EXPR_MODE_UNDEFINED;
        return false;
    }
    return true;
}

string AtomicExpressionSide::getExpr() const
{
    return expr;
}

void AtomicExpressionSide::setExpr(const string &value)
{
    expr = value;
    boost::trim(expr);
}

set<string> AtomicExpressionSide::resolve(const Json::Value &v, bool resolveRegex, bool ignoreCase)
{
    switch (mode)
    {
    case EXPR_MODE_JSONPATH:
    {
        Json::Path path(expr.substr(1));
        Json::Value result = path.resolve(v);
        set<string> res;

        if (result.size() == 0 && !result.isNull())
        {
            res.insert( result.asString() );
        }
        else
        {
            for (size_t i=0; i<result.size();i++)
                res.insert( result[(int)i].asString() );
        }
        return res;
    }
    case EXPR_MODE_STATIC_STRING:
        if (resolveRegex)
        {
            recompileRegex((*staticTexts)[stoul(expr.substr(8))], ignoreCase);
            return {};
        }
        else
            return { (*staticTexts)[stoul(expr.substr(8))] };
    case EXPR_MODE_NUMERIC:
        if (resolveRegex)
        {
            recompileRegex(expr, ignoreCase);
            return {};
        }
        else
            return { expr };
    case EXPR_MODE_NULL:
    case EXPR_MODE_UNDEFINED:
    default:
        return {};
    }
}

boost::regex *AtomicExpressionSide::getRegexp() const
{
    return regexp;
}

void AtomicExpressionSide::setRegexp(boost::regex *value)
{
    regexp = value;
}

eExpressionSideMode AtomicExpressionSide::getMode() const
{
    return mode;
}

set<string> AtomicExpressionSide::recompileRegex(const string &r, bool ignoreCase)
{
    if (!regexp)
    {
        regexp = new boost::regex(r.c_str(),
                                  ignoreCase? (boost::regex::extended|boost::regex::icase) : (boost::regex::extended) );
    }
    return {};
}
