#include "atomicexpression.h"
#include <boost/regex.hpp>
#include <string>
#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>


using namespace std;

AtomicExpression::AtomicExpression(std::vector<string> *staticTexts) : left(staticTexts), right(staticTexts)
{
    evalOperator = EVAL_OPERATOR_UNDEFINED;
    ignoreCase = false;
    negativeExpression = false;
    setStaticTexts(staticTexts);
}

bool AtomicExpression::compile(std::string expr)
{
    if (boost::starts_with(expr,"!"))
    {
        negativeExpression=true;
        expr = expr.substr(1);
    }
    if (boost::starts_with(expr,"i"))
    {
        ignoreCase=true;
        expr = expr.substr(1);
    }
    this->expr = expr;

    if (substractExpressions("^IS_EQUAL\\((?<LEFT_EXPR>[^,]+),(?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_ISEQUAL))
    {
    }
    else if (substractExpressions("^REGEX_MATCH\\((?<LEFT_EXPR>[^,]+),(?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_REGEXMATCH))
    {
    }
    else if (substractExpressions("^CONTAINS\\((?<LEFT_EXPR>[^,]+),(?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_CONTAINS))
    {
    }
    else if (substractExpressions("^STARTS_WITH\\((?<LEFT_EXPR>[^,]+),(?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_STARTSWITH))
    {
    }
    else if (substractExpressions("^ENDS_WITH\\((?<LEFT_EXPR>[^,]+),(?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_ENDSWITH))
    {
    }
    else if (substractExpressions("^IS_NULL\\((?<RIGHT_EXPR>[^\\)]+)\\)$",EVAL_OPERATOR_ISNULL))
    {
    }
    else
    {
        evalOperator=EVAL_OPERATOR_UNDEFINED;
        negativeExpression=false;
        return false;
    }

    return true;
}

bool AtomicExpression::evaluate(const Json::Value &values)
{
    std::set<std::string> lvalues = left.resolve(values,evalOperator == EVAL_OPERATOR_REGEXMATCH, ignoreCase);
    std::set<std::string> rvalues = right.resolve(values,evalOperator == EVAL_OPERATOR_REGEXMATCH, ignoreCase);

    switch (evalOperator)
    {
    case EVAL_OPERATOR_UNDEFINED:
        return calcNegative(false);
    case EVAL_OPERATOR_ENDSWITH:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( ignoreCase && boost::iends_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
                else if ( !ignoreCase && boost::ends_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_STARTSWITH:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( ignoreCase && boost::istarts_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
                else if ( !ignoreCase && boost::starts_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_ISEQUAL:
        for ( const std::string & rvalue : rvalues  )
        {
            if ( !ignoreCase && lvalues.find(rvalue) != lvalues.end())
                return calcNegative(true);
            else if (ignoreCase)
            {
                for ( const std::string & lvalue : lvalues  )
                {
                    if ( boost::iequals( rvalue, lvalue  ) )
                        return calcNegative(true);
                }
            }
        }
        return calcNegative(false);
    case EVAL_OPERATOR_ISNULL:
        return calcNegative(lvalues.empty());
    case EVAL_OPERATOR_CONTAINS:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if (  !ignoreCase && boost::contains(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
                else if ( ignoreCase && boost::icontains(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_REGEXMATCH:
        boost::cmatch what;
        // Regex, any of.
        for ( const std::string & lvalue : lvalues )
        {
            if(right.getRegexp() && boost::regex_match(lvalue.c_str(), what, *right.getRegexp()))
            {
                return calcNegative(true);
            }
        }
        return calcNegative(false);
    }
    return calcNegative(false);
}

bool AtomicExpression::calcNegative(bool r)
{
    if (negativeExpression) return !r;
    return r;
}

bool AtomicExpression::substractExpressions(const std::string &regex, const eEvalOperator &op)
{
    boost::regex exOperatorEqual(regex);
    boost::match_results<string::const_iterator> whatDataDecomposed;
    boost::match_flag_type flags = boost::match_default;

    for (string::const_iterator start = expr.begin(), end = expr.end();
         boost::regex_search(start, end, whatDataDecomposed, exOperatorEqual, flags);
         start = whatDataDecomposed[0].second)
    {
        left.setExpr(string(whatDataDecomposed[1].first, whatDataDecomposed[1].second));
        if ( op!=EVAL_OPERATOR_ISNULL )
            right.setExpr(string(whatDataDecomposed[2].first, whatDataDecomposed[2].second));
        else
            right.setExpr("");

        if (!left.calcMode())
            return false;
        if (!right.calcMode())
            return false;

        evalOperator=op;
        return true;
    }
    return false;
}

void AtomicExpression::setStaticTexts(std::vector<std::string> *value)
{
    staticTexts = value;
}
