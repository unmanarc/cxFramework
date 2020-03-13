#include "iauth_session.h"
#include <random>
#include <iomanip>
#include <sstream>

IAuth_Session::IAuth_Session()
{
    setAuthenticator(nullptr);
    regenSessionId();
}

AuthReason IAuth_Session::isAuthenticated(uint32_t passIndex)
{
    std::unique_lock<std::mutex> lock(mutexAuth);

    sCurrentAuthentication curAuth = getCurrentAuthentication(passIndex);
    if (curAuth.lastAuthReason == AUTH_REASON_AUTHENTICATED)
    {
        // If authenticated: check policy
        if (authPolicies.find(passIndex)!=authPolicies.end())
        {
           if (authPolicies[passIndex].validTime>0 && curAuth.authTime+authPolicies[passIndex].validTime < time(nullptr)) return AUTH_REASON_EXPIRED;
        }
    }
    return curAuth.lastAuthReason;
}

AuthReason IAuth_Session::authenticate(const std::string &accountName, const std::string &domainName, const std::string &password, uint32_t passIndex, AuthMode authMode, const std::string &cramSalt)
{
    std::unique_lock<std::mutex> lock(mutexAuth);

    if (!authenticator)
        return AUTH_REASON_INTERNAL_ERROR;

    AuthReason reason = authenticator->authenticate(accountName,domainName,password,passIndex,authMode,cramSalt);
    authMatrix[passIndex].lastAuthReason = reason;
    authMatrix[passIndex].setCurrentTime();

    if (reason == AUTH_REASON_AUTHENTICATED)
    {
        updateLastActivity();
        firstActivity = lastActivity;
    }
    // Authenticated with the main password:
    if (!passIndex && reason==AUTH_REASON_AUTHENTICATED)
    {
        authUser = accountName;
        authDomain = domainName;
    }
    return reason;
}

void IAuth_Session::updateLastActivity()
{
    lastActivity = time(nullptr);
}

bool IAuth_Session::isLastActivityExpired(const uint32_t &expSeconds) const
{
    return (time(nullptr)-lastActivity)>expSeconds;
}

time_t IAuth_Session::getLastActivity() const
{
    return lastActivity;
}

void IAuth_Session::regenSessionId()
{
    // TODO: check randomness for crypto!
    std::stringstream stream;
    std::random_device rd;
    std::mt19937_64 eng1(rd());
    std::uniform_int_distribution<uint64_t> distr;
    stream << std::uppercase << std::hex << distr( eng1 );
    stream << std::uppercase << std::hex << distr( eng1 );
    sessionId = stream.str();
}

std::string IAuth_Session::getUserID()
{
    return authUser + "/" + authDomain;
}

sCurrentAuthentication IAuth_Session::getCurrentAuthentication(const uint32_t &passIndex)
{
    if (authMatrix.find(passIndex)!=authMatrix.end())
        return authMatrix[passIndex];
    sCurrentAuthentication x;
    return x;
}

void IAuth_Session::setLastActivity(const time_t &value)
{
    lastActivity = value;
}

void IAuth_Session::setAuthUser(const std::string &value)
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    authUser = value;
}

time_t IAuth_Session::getFirstActivity()
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    return firstActivity;
}

std::string IAuth_Session::getSessionId()
{ 
    std::unique_lock<std::mutex> lock(mutexAuth);
    return sessionId;
}

void IAuth_Session::setSessionId(const std::string &value)
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    sessionId = value;
}

IAuth *IAuth_Session::getAuthenticator()
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    return authenticator;
}

std::string IAuth_Session::getAuthUser()
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    return authUser;
}

void IAuth_Session::setIndexAuthenticationPolicy(uint32_t passIndex, const sAuthenticationPolicy &authPolicy)
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    authPolicies[passIndex] = authPolicy;
}

void IAuth_Session::setAuthenticator(IAuth *value)
{
    std::unique_lock<std::mutex> lock(mutexAuth);
    authenticator = value;
}
