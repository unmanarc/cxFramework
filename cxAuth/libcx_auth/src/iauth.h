#ifndef IAUTH_H
#define IAUTH_H

#include "iauth_validation_account.h"
#include <list>
#include <set>
#include <time.h>
#include <cx_thr_mutex/lock_mutex_shared.h>

class IAuth : public IAuth_Validation_Account
{
public:
    IAuth();
    virtual ~IAuth() override;

    virtual bool initScheme()=0;
    virtual bool initAccounts();

    /////////////////////////////////////////////////////////////////////////////////
    // authentication:
    AuthReason authenticate(const std::string & accountName, const std::string & domainName, const std::string & password, uint32_t passIndex = 0, AuthMode authMode = AUTH_MODE_PLAIN, const std::string & cramSalt = "") override;
    sPasswordBasicData accountPasswordBasicData(const std::string & accountName, bool * found, uint32_t passIndex=0, const std::string & domainName = "") override;

    /////////////////////////////////////////////////////////////////////////////////
    // domains:
    virtual bool domainAdd(const std::string & domainName = "")=0;
    virtual bool domainRemove(const std::string & domainName)=0;
    virtual std::list<std::string> domainList()=0;

    /////////////////////////////////////////////////////////////////////////////////
    // account:
    virtual     bool accountAdd(const std::string & accountName,
                                const sPasswordData &passData,
                                const std::string & domainName = "",
                                const std::string & email = "",
                                const std::string & accountDescription = "",
                                const std::string & extraData = "",
                                time_t expirationDate = 0,
                                bool enabled = true,
                                bool confirmed = true,
                                bool superuser = false)=0;

    virtual bool accountChangePassword(const std::string & accountName, const sPasswordData & passwordData, uint32_t passIndex=0, const std::string & domainName = "")=0;
    virtual bool accountRemove(const std::string & accountName, const std::string & domainName = "")=0;
    virtual bool accountDisable(const std::string & accountName, const std::string & domainName = "", bool disabled = true)=0;
    virtual bool accountConfirm(const std::string & accountName, const std::string & confirmationToken, const std::string & domainName = "")=0;
    virtual bool accountChangeDescription(const std::string & accountName, const std::string & description, const std::string & domainName = "")=0;
    virtual bool accountChangeEmail(const std::string & accountName, const std::string & email, const std::string & domainName = "")=0;
    virtual bool accountChangeExtraData(const std::string & accountName, const std::string & extraData, const std::string & domainName = "")=0;
    virtual bool accountChangeExpiration(const std::string & accountName, const std::string & domainName = "", time_t expiration = 0)=0;
    virtual bool isAccountDisabled(const std::string & accountName, const std::string & domainName = "")=0;
    virtual bool isAccountConfirmed(const std::string & accountName, const std::string & domainName = "")=0;
    virtual bool isAccountSuperUser(const std::string & accountName, const std::string & domainName = "")=0;
    virtual std::string accountDescription(const std::string & accountName, const std::string & domainName = "")=0;
    virtual std::string accountEmail(const std::string & accountName, const std::string & domainName = "")=0;
    virtual std::string accountExtraData(const std::string & accountName, const std::string & domainName = "")=0;
    virtual time_t accountExpirationDate(const std::string & accountName, const std::string & domainName = "")=0;
    bool isAccountExpired(const std::string & accountName, const std::string & domainName = "");
    bool accountValidateAttribute(const std::string & accountName, const std::string & attribName, std::string domainName = "") override;
    virtual std::list<std::string> accountsList(const std::string & domainName = "")=0;
    virtual std::list<std::string> accountGroups(const std::string & accountName, const std::string & domainName = "", bool lock = true)=0;
    virtual std::list<std::string> accountDirectAttribs(const std::string & accountName, const std::string & domainName = "", bool lock = true)=0;
    std::set<std::string> accountUsableAttribs(const std::string & accountName, const std::string & domainName = "");

    /////////////////////////////////////////////////////////////////////////////////
    // attributes:
    virtual bool attribAdd(const std::string & attribName, const std::string & attribDescription, std::string domainName = "")=0;
    virtual bool attribRemove(const std::string & attribName, const std::string & domainName = "")=0;
    virtual bool attribGroupAdd(const std::string & attribName, const std::string & groupName, const std::string & domainName = "")=0;
    virtual bool attribGroupRemove(const std::string & attribName, const std::string & groupName, const std::string & domainName = "", bool lock = true)=0;
    virtual bool attribAccountAdd(const std::string & attribName, const std::string & accountName, const std::string & domainName = "")=0;
    virtual bool attribAccountRemove(const std::string & attribName, const std::string & accountName, const std::string & domainName = "", bool lock = true)=0;
    virtual bool attribChangeDescription(const std::string & attribName, const std::string & attribDescription, const std::string & domainName = "")=0;
    virtual std::list<std::string> attribsList(const std::string & domainName = "")=0;
    virtual std::list<std::string> attribGroups(const std::string & attribName, const std::string & domainName, bool lock = true)=0;
    virtual std::list<std::string> attribAccounts(const std::string & attribName, const std::string & domainName, bool lock = true)=0;

    /////////////////////////////////////////////////////////////////////////////////
    // group:
    virtual bool groupAdd(const std::string & groupName, const std::string & groupDescription, const std::string &domainName = "")=0;
    virtual bool groupRemove(const std::string & groupName, const std::string & domainName = "")=0;
    virtual bool groupExist(const std::string & groupName, const std::string & domainName = "")=0;
    virtual bool groupAccountAdd(const std::string & groupName, const std::string & accountName, const std::string & domainName = "")=0;
    virtual bool groupAccountRemove(const std::string & groupName, const std::string & accountName, const std::string & domainName = "", bool lock = true)=0;
    virtual bool groupChangeDescription(const std::string & groupName, const std::string & groupDescription, const std::string & domainName = "")=0;
    virtual bool groupValidateAttribute(const std::string & groupName, const std::string & attribName, std::string domainName = "", bool lock =true)=0;
    virtual std::string groupDescription(const std::string & groupName, const std::string & domainName = "")=0;
    virtual std::list<std::string> groupsList(const std::string & domainName = "")=0;
    virtual std::list<std::string> groupAttribs(const std::string & groupName, const std::string & domainName, bool lock = true)=0;
    virtual std::list<std::string> groupAccounts(const std::string & groupName, const std::string & domainName, bool lock = true)=0;

protected:
    virtual bool accountValidateDirectAttribute(const std::string & accountName, const std::string & attribName, std::string domainName)=0;
    std::string genRandomConfirmationToken();
    virtual sPasswordData retrievePassword(const std::string &accountName, const std::string &domainName, uint32_t passIndex, bool * found)=0;


    Mutex_RW mutex;
    std::string appName;
    std::string workingAuthDir;
};

#endif // IAUTH_H
