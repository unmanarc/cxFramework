#ifndef JSONEXPREVAL_H
#define JSONEXPREVAL_H

#include "atomicexpression.h"

enum eEvalMode {
    EVAL_MODE_OR,
    EVAL_MODE_AND,
    EVAL_MODE_UNDEFINED
};

class JSONExprEval
{
public:
    JSONExprEval();
    JSONExprEval(const std::string & expr);
    JSONExprEval(const std::string & expr, std::vector<std::string> * staticTexts, bool negativeExpression);
    ~JSONExprEval();

    bool evaluate( const Json::Value & values );

    std::string getLastCompilerError() const;

private:
    bool compile( std::string expr );

    bool calcNegative(bool r);

    /**
     * @brief detectSubExpr Detect and replace sub expression
     * @param expr expression string (without quotes)
     * @return -1 if failed, 0 if succeed, 1 if no more expressions
     */
    int detectSubExpr(std::string & expr);

    std::string expression, lastError;
    std::vector<std::string> * staticTexts;
    std::vector<JSONExprEval *> subExpressions;
    std::vector<std::pair<AtomicExpression *,size_t>> atomExpressions;

    bool staticTextsOwner;
    bool negativeExpression;

    eEvalMode evalMode;
};

#endif // JSONEXPREVAL_H
