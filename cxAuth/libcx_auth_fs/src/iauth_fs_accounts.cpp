#include "iauth_fs.h"

#include "iauth_fs_helper_jsonfileaccess.h"

#include <cx_auth/hlp_auth_strings.h>
#include <boost/filesystem.hpp>

#include <sys/stat.h>
#include <sys/types.h>

bool IAuth_FS::_pAccountGroupsDir(const std::string &accountDir, std::string &accountDirGroupsOut)
{
    accountDirGroupsOut = accountDir + "/groups";
    return !access(accountDirGroupsOut.c_str(), W_OK);
}

bool IAuth_FS::_pAccountAttribsDir(const std::string &accountDir, std::string &accountDirAttribsOut)
{
    accountDirAttribsOut = accountDir + "/attribs";
    return !access(accountDirAttribsOut.c_str(), W_OK);
}

bool IAuth_FS::_pAccountExist(const std::string &accountName, const std::string &domainName)
{
    if (workingAuthDir.empty()) return false;
    std::string accountDir;
    return _pAccountDir(accountName, domainName, accountDir);
}

bool IAuth_FS::_pAccountDirCreate(const std::string &accountName, std::string domainName, std::string &accountDirOut)
{
    std::string domainDir;
    if (!_pDomainDir(domainName, domainDir)) return false;
    std::string accountDir = domainDir + "/accounts/" + toURLEncoded(accountName);
    if (access(accountDir.c_str(), W_OK) && mkdir(accountDir.c_str(),0750)) return false;
    accountDirOut = accountDir;
    return true;
}

bool IAuth_FS::_pAccountDir(const std::string &accountName, std::string domainName, std::string &accountDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;

    if (domainName == "") domainName = "default";
    accountDirOut = workingAuthDir + "/"  + toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName);
    return !access(accountDirOut.c_str(),W_OK);
}

bool IAuth_FS::_pAccountsDir(std::string domainName, std::string &accountsDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    if (domainName == "") domainName = "default";
    accountsDirOut = workingAuthDir + "/"  + toURLEncoded(domainName) + "/accounts";
    return !access(accountsDirOut.c_str(),W_OK);
}

std::list<std::string> IAuth_FS::accountsList(const std::string & domainName)
{
    std::list<std::string> accounts;
    Lock_Mutex_RD lock(mutex);
    std::string accountsDir;
    if (_pAccountsDir(domainName,accountsDir))
    {
        accounts = _pListDir(accountsDir);
    }
    
    return accounts;
}

bool IAuth_FS::accountAdd(const std::string &accountName,
                          const sPasswordData & passData,
                          const std::string &domainName,
                          const std::string &email,
                          const std::string &description,
                          const std::string &extraData,
                          time_t expiration,
                          bool enabled,
                          bool confirmed,
                          bool superuser)
{
    std::string accountDir, accountGroupsDir, accountAttribsDir;
    bool r = false;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        r=!_pAccountExist(accountName,domainName);
        if (r) r = _pAccountDirCreate(accountName, domainName,accountDir);
        if (r && (r=_pAccountAttribsDir(accountDir, accountAttribsDir))==false) r=!mkdir(accountAttribsDir.c_str(), 0750);
        if (r && (r=_pAccountGroupsDir(accountDir, accountGroupsDir))==false)   r=!mkdir(accountGroupsDir.c_str(), 0750);
        if (r)
        {
            // Dir created here, now fill the data.
            Json::Value acctDetails, acctPassword, acctActivation;

            acctPassword = passData.getJson();

            acctDetails["email"] = email;
            acctDetails["description"] = description;
            acctDetails["extraData"] = extraData;

            acctActivation["confirmationToken"] = genRandomConfirmationToken();
            acctActivation["enabled"] = enabled;
            acctActivation["confirmed"] = confirmed;
            acctActivation["expiration"] = (unsigned int)expiration;

            saveFile_Activation(accountName,domainName, acctActivation);
            saveFile_Details(accountName,domainName, acctDetails);
            saveFile_Password(accountName,domainName,0,acctPassword);

            if (superuser)
            {
                std::string superUserFile = accountDir + "/superuser";
                _pTouchFile(superUserFile);
            }
        }
    }
    
    return r;
}

bool IAuth_FS::accountRemove(const std::string & accountName, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName, domainName,accountDir))==true)
        {
            // Remove attribs association
            for (const std::string & attribName: accountDirectAttribs(accountName,domainName,false))
                attribAccountRemove(attribName,accountName,domainName,false);

            // Remove group associations
            for (const std::string & groupName: accountGroups(accountName,domainName,false))
                groupAccountRemove(groupName, accountName,domainName,false);

            // Remove the directory.
            boost::filesystem::remove_all(accountDir);
        }
    }
    
    return r;
}

bool IAuth_FS::accountDisable(const std::string &accountName, const std::string & domainName, bool disabled)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName, domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            jAct["enabled"] = !disabled;
            saveFile_Activation(accountName,domainName,jAct);
        }
    }
    
    return r;
}

bool IAuth_FS::accountConfirm(const std::string &accountName, const std::string &confirmationToken, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            if (jAct["confirmationToken"].asString() == confirmationToken)
            {
                jAct["confirmed"] = true;
                saveFile_Activation(accountName,domainName,jAct);
            }
            else r = false; // not confirmed :(
        }
    }
    
    return r;
}

bool IAuth_FS::accountChangePassword(const std::string &accountName, const sPasswordData &passwordData, uint32_t passIndex, const std::string &domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jPasswd;
            jPasswd = passwordData.getJson();
            saveFile_Password(accountName,domainName,passIndex,jPasswd);
        }
    }
    
    return r;
}

bool IAuth_FS::accountChangeDescription(const std::string &accountName, const std::string &description, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            jDetails["description"] = description;
            saveFile_Details(accountName,domainName,jDetails);
        }
    }
    
    return r;
}

bool IAuth_FS::accountChangeEmail(const std::string &accountName, const std::string &email, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            jDetails["email"] = email;
            saveFile_Details(accountName,domainName,jDetails);
        }
    }
    
    return r;
}

bool IAuth_FS::accountChangeExtraData(const std::string &accountName, const std::string &extraData, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            jDetails["extraData"] = extraData;
            saveFile_Details(accountName,domainName,jDetails);
        }
    }
    
    return r;
}

bool IAuth_FS::accountChangeExpiration(const std::string &accountName, const std::string & domainName, time_t expiration)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            jAct["expiration"] = (unsigned int)expiration;
            saveFile_Activation(accountName,domainName,jAct);
        }
    }
    
    return r;
}

bool IAuth_FS::isAccountDisabled(const std::string &accountName, const std::string & domainName)
{
    bool r = true;
    std::string accountDir;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=!_pAccountDir(accountName,domainName,accountDir))==false)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            r = !jAct["enabled"].asBool();
        }
    }
    
    return r;
}

bool IAuth_FS::isAccountConfirmed(const std::string &accountName, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            r = jAct["confirmed"].asBool();
        }
    }
    
    return r;
}

bool IAuth_FS::isAccountSuperUser(const std::string &accountName, const std::string & domainName)
{
    bool r = false;
    std::string accountDir;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAccountDir(accountName,domainName,accountDir))==true)
        {
            std::string superUserFile = accountDir + "/superuser";
            r = !access(superUserFile.c_str(),R_OK);
        }
    }
    
    return r;
}

std::string IAuth_FS::accountDescription(const std::string &accountName, const std::string & domainName)
{
    std::string r;
    std::string accountDir;

    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            r = jDetails["description"].asString();
        }
    }
    
    return r;
}

std::string IAuth_FS::accountEmail(const std::string &accountName, const std::string & domainName)
{
    std::string r;
    std::string accountDir;

    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            r = jDetails["email"].asString();
        }
    }
    
    return r;
}


sPasswordData IAuth_FS::retrievePassword(const std::string &accountName, const std::string &domainName, uint32_t passIndex, bool *found)
{
    sPasswordData r;
    std::string accountDir;

    *found = false;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName,domainName,accountDir))==true)
        {
            *found = true;
            Json::Value jPasswd = loadFile_Password(accountName,passIndex,domainName);
            r.setJson(jPasswd);
        }
    }
    
    return r;
}


std::string IAuth_FS::accountConfirmationToken(const std::string & accountName, const std::string & domainName)
{
    std::string accountDir, r;

    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            r = jAct["confirmationToken"].asString();
        }
    }
    
    return r;
}

std::string IAuth_FS::accountExtraData(const std::string &accountName, const std::string & domainName)
{
    std::string r;
    std::string accountDir;

    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName,domainName,accountDir))==true)
        {
            Json::Value jDetails = loadFile_AccountDetails(accountName,domainName);
            r = jDetails["extraData"].asString();
        }
    }
    
    return r;
}

time_t IAuth_FS::accountExpirationDate(const std::string &accountName, const std::string & domainName)
{
    time_t r = 0;
    std::string accountDir;

    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pAccountDir(accountName, domainName,accountDir))==true)
        {
            Json::Value jAct = loadFile_Activation(accountName,domainName);
            r = jAct["expiration"].asUInt();
        }
    }
    
    return r;
}

Json::Value IAuth_FS::loadFile_Activation(const std::string &accountName, std::string domainName)
{
    if (domainName == "") domainName = "default";
    return loadJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.activation");
}

Json::Value IAuth_FS::loadFile_AccountDetails(const std::string &accountName, std::string domainName)
{
    if (domainName == "") domainName = "default";
    return loadJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.details");
}

Json::Value IAuth_FS::loadFile_Password(const std::string &accountName, uint32_t passIndex, std::string domainName)
{
    if (domainName == "") domainName = "default";
    return loadJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.passwd." +  std::to_string(passIndex));
}

bool IAuth_FS::saveFile_Activation(const std::string &accountName, std::string domainName, const Json::Value &v)
{
    if (domainName == "") domainName = "default";
    return saveJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.activation",v);
}

bool IAuth_FS::saveFile_Details(const std::string &accountName, std::string domainName, const Json::Value &v)
{
    if (domainName == "") domainName = "default";
    return saveJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.details",v);
}

bool IAuth_FS::saveFile_Password(const std::string &accountName, std::string domainName, uint32_t passIndex, const Json::Value &v)
{
    if (domainName == "") domainName = "default";
    return saveJSONFile(workingAuthDir  + "/" +  toURLEncoded(domainName) + "/accounts/" + toURLEncoded(accountName) + "/acct.passwd." + std::to_string(passIndex) ,v);
}

