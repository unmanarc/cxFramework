#ifndef IAUTH_VALIDATION_ACCOUNT_H
#define IAUTH_VALIDATION_ACCOUNT_H

#include <string>
#include "ds_auth_modes.h"
#include "ds_auth_reasons.h"
#include "ds_auth_passmodes.h"
#include "ds_auth_passworddata.h"

class IAuth_Validation_Account
{
public:
    IAuth_Validation_Account();
    virtual ~IAuth_Validation_Account();

    static sPasswordData genPassword(const std::string & passwordInput, const PasswordModes & passMode, bool forceExpiration = false, const time_t &expirationDate = std::numeric_limits<time_t>::max(), uint32_t _2faSteps = 0);

    virtual std::string accountConfirmationToken(const std::string & accountName, const std::string & domainName = "")=0;
    virtual sPasswordBasicData accountPasswordBasicData(const std::string & accountName, bool * found, uint32_t passIndex=0, const std::string & domainName = "")=0;
    virtual AuthReason authenticate(const std::string & accountName, const std::string & domainName, const std::string & password, uint32_t passIndex = 0, AuthMode authMode = AUTH_MODE_PLAIN, const std::string & cramSalt = "")=0;

    virtual bool accountValidateAttribute(const std::string & accountName, const std::string & attribName, std::string domainName = "")=0;

protected:
    AuthReason validatePassword(const sPasswordData & storedPassword, const std::string & passwordInput, const std::string &cramSalt, AuthMode authMode);

private:
    static std::string calcSHA256(const std::string & password);
    static std::string calcSHA512(const std::string & password);

    // SALT must be 4 bytes.
    static std::string calcSSHA256(const std::string & password, const unsigned char *ssalt);
    static std::string calcSSHA512(const std::string & password, const unsigned char *ssalt);

    AuthReason validateCRAM(const std::string & passwordFromDB, const std::string & passwordInput, const std::string &cramSalt);
    AuthReason validateGAuth(const std::string & seed, const std::string & token);
};

#endif // IAUTH_VALIDATION_ACCOUNT_H
