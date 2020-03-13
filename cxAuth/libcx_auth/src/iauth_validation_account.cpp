#include "iauth_validation_account.h"
#include "hlp_auth_strings.h"
#include <openssl/sha.h>

IAuth_Validation_Account::IAuth_Validation_Account()
{
}

IAuth_Validation_Account::~IAuth_Validation_Account()
{

}

sPasswordData IAuth_Validation_Account::genPassword(const std::string &passwordInput, const PasswordModes &passMode, bool forceExpiration, const time_t &expirationDate, uint32_t _2faSteps)
{
    sPasswordData r;

    r.passwordMode = passMode;
    r.forceExpiration = forceExpiration;
    r.expiration = expirationDate;

    switch (passMode)
    {
    case PASS_MODE_NOTFOUND:
    {
        // Do nothing...
    } break;
    case PASS_MODE_PLAIN:
    {
        r.hash = passwordInput;
    } break;
    case PASS_MODE_SHA256:
    {
        r.hash = calcSHA256(passwordInput);
    } break;
    case PASS_MODE_SHA512:
    {
        r.hash = calcSHA512(passwordInput);
    } break;
    case PASS_MODE_SSHA256:
    {
        createRandomSalt(r.ssalt);
        r.hash = calcSSHA256(passwordInput, r.ssalt);
    } break;
    case PASS_MODE_SSHA512:
    {
        createRandomSalt(r.ssalt);
        r.hash = calcSSHA512(passwordInput, r.ssalt);
    } break;
    case PASS_MODE_GAUTHTIME:
        r.hash = passwordInput;
        r.gAuthSteps = _2faSteps;
    }

    return r;
}

AuthReason IAuth_Validation_Account::validatePassword(const sPasswordData &storedPassword, const std::string &passwordInput, const std::string &cramSalt, AuthMode authMode)
{
    bool saltedHash = false;
    std::string toCompare;

    if (storedPassword.isPasswordExpired())
        return AUTH_REASON_EXPIRED_PASSWORD;

    switch (storedPassword.passwordMode)
    {
    case PASS_MODE_NOTFOUND:
        return AUTH_REASON_INTERNAL_ERROR;
    case PASS_MODE_PLAIN:
    {
        toCompare = passwordInput;
    } break;
    case PASS_MODE_SHA256:
    {
        toCompare = calcSHA256(passwordInput);
    } break;
    case PASS_MODE_SHA512:
    {
        toCompare = calcSHA512(passwordInput);
    } break;
    case PASS_MODE_SSHA256:
    {
        toCompare = calcSSHA256(passwordInput, storedPassword.ssalt);
        saltedHash = true;
    } break;
    case PASS_MODE_SSHA512:
    {
        toCompare = calcSSHA512(passwordInput, storedPassword.ssalt);
        saltedHash = true;
    } break;
    case PASS_MODE_GAUTHTIME:
        return validateGAuth(storedPassword.hash,passwordInput); // GAuth Time Based Token comparisson (seed,token)
    }

    switch (authMode)
    {
    case AUTH_MODE_PLAIN:
        return storedPassword.hash==toCompare? AUTH_REASON_AUTHENTICATED:AUTH_REASON_BAD_PASSWORD; // 1-1 comparisson
    case AUTH_MODE_CRAM:
        return saltedHash?AUTH_REASON_INTERNAL_ERROR:validateCRAM(storedPassword.hash, passwordInput, cramSalt);
    }

    return AUTH_REASON_NOT_IMPLEMENTED;
}

std::string IAuth_Validation_Account::calcSHA256(const std::string &password)
{
    std::string r;
    unsigned char buffer_sha2[SHA256_DIGEST_LENGTH+1];
    SHA256_CTX sha2;
    SHA256_Init (&sha2);
    SHA256_Update (&sha2, (const unsigned char *) password.c_str(), password.length());
    SHA256_Final ( buffer_sha2, &sha2);
    return strToHex(buffer_sha2,SHA256_DIGEST_LENGTH);
}

std::string IAuth_Validation_Account::calcSHA512(const std::string &password)
{
    std::string r;
    unsigned char buffer_sha2[SHA512_DIGEST_LENGTH+1];
    SHA512_CTX sha2;
    SHA512_Init (&sha2);
    SHA512_Update (&sha2, (const unsigned char *) password.c_str(), password.length());
    SHA512_Final ( buffer_sha2, &sha2);
    return strToHex(buffer_sha2,SHA512_DIGEST_LENGTH);
}

std::string IAuth_Validation_Account::calcSSHA256(const std::string &password, const unsigned char * ssalt)
{
    std::string r;
    unsigned char buffer_sha2[SHA256_DIGEST_LENGTH+1];
    SHA256_CTX sha2;
    SHA256_Init (&sha2);
    SHA256_Update (&sha2, (const unsigned char *) password.c_str(), password.length());
    SHA256_Update (&sha2, ssalt, 4);
    SHA256_Final ( buffer_sha2, &sha2);
    return strToHex(buffer_sha2,SHA256_DIGEST_LENGTH);
}

std::string IAuth_Validation_Account::calcSSHA512(const std::string &password, const unsigned char * ssalt)
{
    std::string r;
    unsigned char buffer_sha2[SHA512_DIGEST_LENGTH+1];
    SHA512_CTX sha2;
    SHA512_Init (&sha2);
    SHA512_Update (&sha2, (const unsigned char *) password.c_str(), password.length());
    SHA512_Update (&sha2, ssalt, 4);
    SHA512_Final ( buffer_sha2, &sha2);
    return strToHex(buffer_sha2,SHA512_DIGEST_LENGTH);
}

AuthReason IAuth_Validation_Account::validateCRAM(const std::string &passwordFromDB, const std::string &passwordInput, const std::string &cramSalt)
{
    // TODO:
    return AUTH_REASON_NOT_IMPLEMENTED;
}

AuthReason IAuth_Validation_Account::validateGAuth(const std::string &seed, const std::string &token)
{
    // TODO:
    return AUTH_REASON_NOT_IMPLEMENTED;
}
