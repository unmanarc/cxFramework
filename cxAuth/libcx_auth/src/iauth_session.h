#ifndef IAUTH_SESSION_H
#define IAUTH_SESSION_H

#include <string>
#include <map>
#include <mutex>
#include <atomic>

#include "iauth.h"

struct sAuthenticationPolicy
{
    sAuthenticationPolicy()
    {
        validTime = 0;
    }

    time_t validTime;
};


struct sCurrentAuthentication
{
    sCurrentAuthentication()
    {
        lastAuthReason = AUTH_REASON_UNAUTHENTICATED;
    }
    void setCurrentTime()
    {
        authTime = time(nullptr);
    }

    AuthReason lastAuthReason;
    time_t authTime;
};


class IAuth_Session
{
public:
    IAuth_Session();
    IAuth_Session& operator=(IAuth_Session &value)
    {
        this->authenticator = value.authenticator;
        this->authUser = value.authUser;
        this->authDomain = value.authDomain;

        this->sessionId = value.sessionId;
        this->authMatrix = value.authMatrix;
        this->authPolicies = value.authPolicies;
        this->firstActivity = value.firstActivity;

        return *this;
    }

    /**
     * @brief setAuthenticator
     * @param value
     */
    void setAuthenticator(IAuth *value);
    /**
     * @brief getAuthenticator
     * @return
     */
    IAuth *getAuthenticator();
    /**
     * @brief getAuthUser
     * @return
     */
    std::string getAuthUser();
    /**
     * @brief setIndexAuthenticationPolicy
     * @param passIndex
     * @param authPolicy
     */
    void setIndexAuthenticationPolicy(uint32_t passIndex, const sAuthenticationPolicy & authPolicy);
    /**
     * @brief isAuthenticated
     * @param passIndex
     * @return
     */
    AuthReason isAuthenticated(uint32_t passIndex = 0);
    /**
     * @brief authenticate Validate account password
     * @param accountName
     * @param domainName
     * @param password
     * @param passIndex
     * @param authMode
     * @param cramSalt
     * @return
     */
    AuthReason authenticate(const std::string & accountName, const std::string & domainName, const std::string & password, uint32_t passIndex = 0, AuthMode authMode = AUTH_MODE_PLAIN, const std::string & cramSalt = "");
    /**
     * @brief updateLastActivity Set las activity to current time
     */
    void updateLastActivity();
    /**
     * @brief isLastActivityExpired given an expiration time in seconds, tell if this session is expired (inactive)
     * @param expSeconds
     * @return true if expired
     */
    bool isLastActivityExpired(const uint32_t & expSeconds) const;
    /**
     * @brief getLastActivity Get the unix time from the last activity
     * @return unix time of last activity
     */
    time_t getLastActivity() const;
    /**
     * @brief setLastActivity Set last activity time (for assign operations), use updateLastActivity for updating this value
     * @param value unix time
     */
    void setLastActivity(const time_t &value);
    /**
     * @brief regenSessionId regenerate the session id using a pseudo-random number
     */
    void regenSessionId();
    /**
     * @brief setSessionId Set the session id string
     * @param value session id
     */
    void setSessionId(const std::string &value);
    /**
     * @brief getSessionId Get the current session ID
     * @return session ID string (in hex value)
     */
    std::string getSessionId();
    /**
     * @brief getUserID Get the user ID
     * @return user id string (username/domain)
     */
    std::string getUserID();
    /**
     * @brief setAuthUser Set/Change the authenticated user
     * @param value user that authenticated
     */
    void setAuthUser(const std::string &value);

    /**
     * @brief getFirstActivity Get the unix time from where you created this session
     * @return unix time
     */
    time_t getFirstActivity();
private:
    sCurrentAuthentication getCurrentAuthentication(const uint32_t &passIndex);

    IAuth * authenticator;
    std::string authUser, authDomain;
    std::string sessionId;
    std::map<uint32_t,sCurrentAuthentication> authMatrix;
    std::map<uint32_t,sAuthenticationPolicy> authPolicies;

    time_t firstActivity;
    std::atomic<time_t> lastActivity;


    std::mutex mutexAuth;
};

#endif // IAUTH_SESSION_H
