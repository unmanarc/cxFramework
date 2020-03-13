#include "iauth_validation_methods.h"

IAuth_Methods_Attributes::IAuth_Methods_Attributes()
{
}

void IAuth_Methods_Attributes::addMethodAttributes(const std::string &methodName, const std::set<std::string> &attribs)
{
    for (const std::string & attrib : attribs)
        methodAttribs.insert(std::make_pair(methodName, attrib));
}

void IAuth_Methods_Attributes::addAttribPassIndexes(const std::string &attribName, const std::set<uint32_t> &passIndexes)
{
    for (const uint32_t & passIndex : passIndexes)
        attribPassIndexes.insert(std::make_pair(attribName, passIndex));
}

bool IAuth_Methods_Attributes::validateMethod(IAuth_Session *authSession,  const std::string &methodName, const std::set<uint32_t> & extraTmpIndexes, std::set<uint32_t> *passIndexesLeft, std::set<std::string> *attribsLeft)
{

    std::set<uint32_t> requiredPassIndexes  = getMethodPassIndexes(methodName);
    std::set<std::string> requiredAttribs = getMethodAttribs(methodName);

    // start with all...
    *passIndexesLeft = requiredPassIndexes;
    *attribsLeft = requiredAttribs;

    // Validate the required pass indexes for attribs/method
    for ( const uint32_t & passIndex : requiredPassIndexes )
    {
        if ( authSession->isAuthenticated(passIndex) == AUTH_REASON_AUTHENTICATED )
        {
            passIndexesLeft->erase(passIndex);
        }
        if ( extraTmpIndexes.find(passIndex) != extraTmpIndexes.end() )
        {
            passIndexesLeft->erase(passIndex);
        }
    }

    for ( const std::string & attrib : requiredAttribs )
    {
        if (authSession->getAuthenticator()->accountValidateAttribute(authSession->getAuthUser(), attrib))
            attribsLeft->erase(attrib);
    }

    return passIndexesLeft->empty() && attribsLeft->empty();
}

std::set<uint32_t> IAuth_Methods_Attributes::getAttribPassIndexes(const std::string &attribName)
{
    std::set<uint32_t> r = {0};
    auto it = attribPassIndexes.equal_range(attribName);
    for (auto itr = it.first; itr != it.second; ++itr)
        r.insert(itr->second);
    return r;
}

std::set<std::string> IAuth_Methods_Attributes::getMethodAttribs(const std::string &methodName)
{
    std::set<std::string> r;
    auto it = methodAttribs.equal_range(methodName);
    for (auto itr = it.first; itr != it.second; ++itr)
        r.insert(itr->second);
    return r;
}

std::set<uint32_t> IAuth_Methods_Attributes::getMethodPassIndexes(const std::string &methodName)
{
    std::set<uint32_t> r;
    std::set<std::string> attribs= getMethodAttribs(methodName);
    for (const std::string & attribName : attribs)
    {
        std::set<uint32_t> passIndexes = getAttribPassIndexes(attribName);

        for (const uint32_t & passIndex : passIndexes )
        {
            r.insert(passIndex);
        }
    }
    return r;
}
