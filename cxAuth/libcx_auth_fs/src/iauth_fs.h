#ifndef IAUTH_FS_H
#define IAUTH_FS_H

#include <cx_auth/iauth.h>
#include <json/json.h>

// TODO: use the mutex.

class IAuth_FS : public IAuth
{
public:
    // Open authentication system:
    IAuth_FS(const std::string & appName);

    bool initScheme() override;

    /////////////////////////////////////////////////////////////////////////////////
    // domains:
    bool domainAdd(const std::string & domainName = "") override;
    bool domainRemove(const std::string & domainName) override;
    std::list<std::string> domainList() override;

    /////////////////////////////////////////////////////////////////////////////////
    // account:
    bool accountAdd(const std::string & accountName,
                    const sPasswordData &passData,
                    const std::string & domainName = "",
                    const std::string & email = "",
                    const std::string & accountDescription = "",
                    const std::string & extraData = "",
                    time_t expirationDate = 0,
                    bool enabled = true,
                    bool confirmed = true,
                    bool superuser = false) override;
    std::string accountConfirmationToken(const std::string & accountName, const std::string & domainName = "") override;
    bool accountRemove(const std::string & accountName, const std::string & domainName = "") override;
    bool accountDisable(const std::string & accountName, const std::string & domainName = "", bool disabled = true) override;
    bool accountConfirm(const std::string & accountName, const std::string & confirmationToken, const std::string & domainName = "") override;
    bool accountChangePassword(const std::string & accountName, const sPasswordData & passwordData, uint32_t passIndex=0, const std::string & domainName = "") override;
    bool accountChangeDescription(const std::string & accountName, const std::string & description, const std::string & domainName = "") override;
    bool accountChangeEmail(const std::string & accountName, const std::string & email, const std::string & domainName = "") override;
    bool accountChangeExtraData(const std::string & accountName, const std::string & extraData, const std::string & domainName = "") override;
    bool accountChangeExpiration(const std::string & accountName, const std::string & domainName = "", time_t expiration = 0) override;
    bool isAccountDisabled(const std::string & accountName, const std::string & domainName = "") override;
    bool isAccountConfirmed(const std::string & accountName, const std::string & domainName = "") override;
    bool isAccountSuperUser(const std::string & accountName, const std::string & domainName = "") override;
    std::string accountDescription(const std::string & accountName, const std::string & domainName = "") override;
    std::string accountEmail(const std::string & accountName, const std::string & domainName = "") override;
    std::string accountExtraData(const std::string & accountName, const std::string & domainName = "") override;
    time_t accountExpirationDate(const std::string & accountName, const std::string & domainName = "") override;
    std::list<std::string> accountsList(const std::string & domainName = "") override;
    std::list<std::string> accountGroups(const std::string & accountName, const std::string & domainName = "", bool lock = true) override;
    std::list<std::string> accountDirectAttribs(const std::string & accountName, const std::string & domainName = "", bool lock = true) override;

    /////////////////////////////////////////////////////////////////////////////////
    // attributes:
    bool attribAdd(const std::string & attribName, const std::string & attribDescription, std::string domainName = "") override;
    bool attribRemove(const std::string & attribName, const std::string & domainName = "") override;
    bool attribGroupAdd(const std::string & attribName, const std::string & groupName, const std::string & domainName = "") override;
    bool attribGroupRemove(const std::string & attribName, const std::string & groupName, const std::string & domainName = "", bool lock = true) override;
    bool attribAccountAdd(const std::string & attribName, const std::string & accountName, const std::string & domainName = "") override;
    bool attribAccountRemove(const std::string & attribName, const std::string & accountName, const std::string & domainName = "", bool lock = true) override;
    bool attribChangeDescription(const std::string & attribName, const std::string & attribDescription, const std::string & domainName = "") override;
    std::list<std::string> attribsList(const std::string & domainName = "") override;
    std::list<std::string> attribGroups(const std::string & attribName, const std::string & domainName, bool lock = true) override;
    std::list<std::string> attribAccounts(const std::string & attribName, const std::string & domainName, bool lock = true) override;

    /////////////////////////////////////////////////////////////////////////////////
    // group:
    bool groupAdd(const std::string & groupName, const std::string & groupDescription, const std::string &domainName = "") override;
    bool groupRemove(const std::string & groupName, const std::string & domainName = "") override;
    bool groupExist(const std::string & groupName, const std::string & domainName = "") override;
    bool groupAccountAdd(const std::string & groupName, const std::string & accountName, const std::string & domainName = "") override;
    bool groupAccountRemove(const std::string & groupName, const std::string & accountName, const std::string & domainName = "", bool lock = true) override;
    bool groupChangeDescription(const std::string & groupName, const std::string & groupDescription, const std::string & domainName = "") override;
    bool groupValidateAttribute(const std::string & groupName, const std::string & attribName, std::string domainName = "", bool lock =true) override;
    std::string groupDescription(const std::string & groupName, const std::string & domainName = "") override;
    std::list<std::string> groupsList(const std::string & domainName = "") override;
    std::list<std::string> groupAttribs(const std::string & groupName, const std::string & domainName, bool lock = true) override;
    std::list<std::string> groupAccounts(const std::string & groupName, const std::string & domainName, bool lock = true) override;

protected:
    bool accountValidateDirectAttribute(const std::string & accountName, const std::string & attribName, std::string domainName) override;
    sPasswordData retrievePassword(const std::string &accountName, const std::string &domainName, uint32_t passIndex, bool * found) override;

private:
    Json::Value loadFile_Activation(const std::string & accountName, std::string domainName);
    Json::Value loadFile_AccountDetails(const std::string & accountName, std::string domainName);
    Json::Value loadFile_Password(const std::string & accountName, uint32_t passIndex, std::string domainName);

    bool saveFile_Activation(const std::string & accountName, std::string domainName, const Json::Value & v);
    bool saveFile_Details(const std::string & accountName, std::string domainName, const Json::Value & v);
    bool saveFile_Password(const std::string & accountName, std::string domainName, uint32_t passIndex, const Json::Value & v);

    bool saveFile_GroupDetails(const std::string & group, std::string domainName, const Json::Value & v);
    Json::Value loadFile_GroupDetails(const std::string &groupName, std::string domainName);

    Json::Value loadFile_AttribDetails(const std::string &attribName, std::string domainName);
    bool saveFile_AttribDetails(const std::string & attribName, std::string domainName, const Json::Value & v);

    bool _pAccountDirCreate(const std::string &accountName, std::string domainName, std::string &accountDirOut);
    bool _pAccountsDir(std::string domainName, std::string &accountsDirOut);
    bool _pAccountDir(const std::string &accountName, std::string domainName, std::string &accountDirOut);
    bool _pAccountGroupsDir(const std::string &accountDir, std::string &accountDirGroupsOut);
    bool _pAccountAttribsDir(const std::string &accountDir, std::string &accountDirAttribsOut);
    bool _pAccountExist(const std::string &accountName, const std::string &domainName);

    bool _pGroupExist(const std::string & groupName, const std::string &domainName);
    bool _pGroupsDir(std::string domainName, std::string &accountsDirOut);
    bool _pGroupDir(const std::string & groupName, std::string domainName, std::string & groupDirOut);
    bool _pGroupDirCreate(const std::string &groupName, std::string domainName, std::string &groupsDirOut);
    bool _pGroupAccountsDir(const std::string & groupDir, std::string & groupAccountsOut);
    bool _pGroupAttribsDir(const std::string &groupDir, std::string &groupAttribsOut);

    bool _pAttribDirCreate(const std::string &attribName, std::string domainName, std::string &attribsDirOut);
    bool _pAttribsDir(std::string domainName, std::string &accountsDirOut);
    bool _pAttribDir(const std::string &attribName, std::string domainName, std::string &attribDirOut);
    bool _pAttribAccountsDir(const std::string &attribDir, std::string &attribAccountsOut);
    bool _pAttribGroupsDir(const std::string &attribDir, std::string &attribGroupsDirOut);

    bool _pDomainDir(std::string domainName, std::string &domainDirOut);
    bool _pDomainDirCreate(std::string domainName, std::string &domainDirOut);

    bool _pTouchFile(const std::string & fileName, const std::string & value = "1");
    bool _pRemFile(const std::string & fileName);
    std::list<std::string> _pListDir(const std::string &dirPath);

    std::string appName;
    std::string workingAuthDir;
};


#endif // IAUTH_FS_H
