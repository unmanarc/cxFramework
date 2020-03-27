#include "atomicexpression.h"
#include <boost/regex.hpp>
#include <string>
#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>


using namespace std;

AtomicExpression::AtomicExpression(std::vector<string> *staticTexts) : left(staticTexts), right(staticTexts)
{
    evalOperator = EVAL_OPERATOR_UNDEFINED;
    negativeExpression = false;
    setStaticTexts(staticTexts);
}

bool AtomicExpression::compile(std::string expr)
{
    this->expr = expr;
    if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_EQUAL;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_EQUAL;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=I\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_EQUAL_ICASE;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=I\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_EQUAL_ICASE;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=RE\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_REGEX;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=RE\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_REGEX;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=C\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_CONTAIN;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=C\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_CONTAIN;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=CI\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_CONTAIN_ICASE;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=CI\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_CONTAIN_ICASE;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=SW\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_STARTSWITH;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=SW\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_STARTSWITH;
        negativeExpression=true;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\= ]*)\\=SWI\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_STARTSWITH_ICASE;
        negativeExpression=false;
    }
    else if (substractExpressions("^(?<LEFT_EXPR>[^\\! ]*)\\!\\=SWI\\=(?<RIGHT_EXPR>[^ ]*)$"))
    {
        evalOperator=EVAL_OPERATOR_STARTSWITH_ICASE;
        negativeExpression=true;
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
    std::set<std::string> lvalues = left.resolve(values,evalOperator == EVAL_OPERATOR_REGEX);
    std::set<std::string> rvalues = right.resolve(values,evalOperator == EVAL_OPERATOR_REGEX);

    switch (evalOperator)
    {
    case EVAL_OPERATOR_UNDEFINED:
        return calcNegative(false);
    case EVAL_OPERATOR_STARTSWITH:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( boost::starts_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_STARTSWITH_ICASE:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( boost::istarts_with(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_EQUAL_ICASE:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( boost::iequals(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_EQUAL:
        for ( const std::string & rvalue : rvalues  )
        {
            if (lvalues.find(rvalue) != lvalues.end())
                return calcNegative(true);
        }
        return calcNegative(false);
    case EVAL_OPERATOR_CONTAIN:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( boost::contains(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_CONTAIN_ICASE:
        for ( const std::string & lvalue : lvalues  )
        {
            for ( const std::string & rvalue : rvalues  )
            {
                if ( boost::icontains(lvalue,rvalue) )
                {
                    return calcNegative(true);
                }
            }
        }
        return calcNegative( false );
    case EVAL_OPERATOR_REGEX:
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

    /*

    switch (evalOperator)
    {
    case EVAL_OPERATOR_UNDEFINED:
        return calcNegative(false);
    case EVAL_OPERATOR_STARTSWITH:
        return calcNegative( lvalues == rvalues );
    case EVAL_OPERATOR_EQUAL:
        return calcNegative( lvalues == rvalues );
    case EVAL_OPERATOR_CONTAIN:
        for ( const std::string & rvalue : rvalues  )
        {
            if (lvalues.find(rvalue) == lvalues.end()) return calcNegative(false);
        }
        return calcNegative(true);
    case EVAL_OPERATOR_REGEX:
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
*/
    return calcNegative(false);
}

bool AtomicExpression::calcNegative(bool r)
{
    if (negativeExpression) return !r;
    return r;
}

bool AtomicExpression::substractExpressions(const std::string &regex)
{
    boost::regex exOperatorEqual(regex);
    boost::match_results<string::const_iterator> whatDataDecomposed;
    boost::match_flag_type flags = boost::match_default;

    for (string::const_iterator start = expr.begin(), end = expr.end();
         boost::regex_search(start, end, whatDataDecomposed, exOperatorEqual, flags);
         start = whatDataDecomposed[0].second)
    {
        left.setExpr(string(whatDataDecomposed[1].first, whatDataDecomposed[1].second));
        right.setExpr(string(whatDataDecomposed[2].first, whatDataDecomposed[2].second));

        if (!left.calcMode())
            return false;
        if (!right.calcMode())
            return false;

        return true;
    }
    return false;
}

void AtomicExpression::setStaticTexts(std::vector<std::string> *value)
{
    staticTexts = value;
}
