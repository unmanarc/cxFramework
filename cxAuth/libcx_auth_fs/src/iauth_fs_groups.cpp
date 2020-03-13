#include "iauth_fs.h"

#include <cx_auth/hlp_auth_strings.h>
#include "iauth_fs_helper_jsonfileaccess.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>

bool IAuth_FS::_pGroupExist(const std::string &groupName, const std::string &domainName)
{
    if (workingAuthDir.empty()) return false;
    std::string groupDir;
    return _pGroupDir(groupName,domainName,groupDir);
}

bool IAuth_FS::_pGroupDirCreate(const std::string &groupName, std::string domainName, std::string &groupsDirOut)
{
    std::string domainDir;
    if (!_pDomainDir(domainName, domainDir)) return false;
    std::string groupsDir = domainDir + "/groups/" + toURLEncoded(groupName);
    if (access(groupsDir.c_str(), W_OK) && mkdir(groupsDir.c_str(),0750)) return false;
    groupsDirOut = groupsDir;
    return true;
}

bool IAuth_FS::_pGroupDir(const std::string &groupName, std::string domainName, std::string &groupDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    if (domainName == "") domainName = "default";

    groupDirOut = workingAuthDir + "/" + toURLEncoded(domainName) + "/groups/" + toURLEncoded(groupName);
    return !access(groupDirOut.c_str(),W_OK);
}

bool IAuth_FS::_pGroupAccountsDir(const std::string &groupDir, std::string &groupAccountsOut)
{
    groupAccountsOut = groupDir + "/accounts";
    return !access(groupAccountsOut.c_str(), W_OK);
}

bool IAuth_FS::_pGroupAttribsDir(const std::string &groupDir, std::string &groupAttribsOut)
{
    groupAttribsOut = groupDir + "/attribs";
    return !access(groupAttribsOut.c_str(), W_OK);
}

bool IAuth_FS::_pGroupsDir(std::string domainName, std::string &accountsDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    if (domainName == "") domainName = "default";
    accountsDirOut = workingAuthDir + "/"  + toURLEncoded(domainName) + "/attribs";
    return !access(accountsDirOut.c_str(),W_OK);
}

std::list<std::string> IAuth_FS::groupsList(const std::string & domainName)
{
    std::list<std::string> groups;
    Lock_Mutex_RD lock(mutex);
    std::string accountsDir;
    if (_pGroupsDir(domainName,accountsDir))
    {
        groups = _pListDir(accountsDir);
    }
    
    return groups;
}

bool IAuth_FS::groupAdd(const std::string &groupName, const std::string &groupDescription, const std::string &domainName)
{
    std::string groupDir, groupAccountsDir, groupAttribsDir;
    bool r=false;

    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        r = !_pGroupExist(groupName,domainName);
        if (r) r = _pGroupDirCreate(groupName,domainName,groupDir);
        if (r && (r=_pGroupAccountsDir(groupDir, groupAccountsDir))==false)
            r=!mkdir(groupAccountsDir.c_str(), 0750);
        if (r && (r=_pGroupAttribsDir(groupDir, groupAttribsDir))==false)
            r=!mkdir(groupAttribsDir.c_str(), 0750);
        if (r)
        {
            // Dir created here, now fill the data.
            Json::Value groupDetails;
            groupDetails["description"] = groupDescription;
            saveFile_GroupDetails(groupName, domainName,groupDetails);
        }
    }
    
    return r;
}

bool IAuth_FS::groupRemove(const std::string &groupName, const std::string &domainName)
{
    std::string groupDir;
    bool r = false;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pGroupDir(groupName,domainName,groupDir))==true)
        {
            // Remove from accounts association.
            for (const std::string & accountName : groupAccounts(groupName,domainName,false))
                groupAccountRemove(groupName,accountName, domainName,false);

            // Remove from attribs association.
            for (const std::string & attribName : groupAttribs(groupName,domainName,false))
                attribGroupRemove(attribName,groupName,domainName,false);

            // Remove the directory.
            boost::filesystem::remove_all(groupDir);
        }
    }
    
    return r;
}

bool IAuth_FS::groupExist(const std::string &groupName, const std::string &domainName)
{
    bool r = false;
    Lock_Mutex_RD lock(mutex);
    r = _pGroupExist(groupName,domainName);
    
    return r;
}

bool IAuth_FS::groupAccountAdd(const std::string &groupName, const std::string &accountName, const std::string & domainName)
{
    std::string accountDir, accountGroupsDir;
    std::string groupDir, groupAccountsDir;
    bool r=false;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        r=_pGroupDir(groupName,domainName,groupDir) &&
          _pAccountDir(accountName,domainName,accountDir) &&
          _pGroupAccountsDir(groupDir, groupAccountsDir) &&
          _pAccountGroupsDir(accountDir, accountGroupsDir) &&
          _pTouchFile(groupAccountsDir + "/" + toURLEncoded(accountName)) &&
          _pTouchFile(accountGroupsDir + "/" + toURLEncoded(groupName));
    }
    
    return r;
}

bool IAuth_FS::groupAccountRemove(const std::string &groupName, const std::string &accountName, const std::string & domainName, bool lock)
{
    std::string accountDir, accountGroupsDir;
    std::string groupDir, groupAccountsDir;
    bool r=false;
    if (lock) mutex.lock();
    if (!workingAuthDir.empty())
    {
        r=_pGroupDir(groupName,domainName,groupDir) &&
          _pAccountDir(accountName,domainName,accountDir) &&
          _pGroupAccountsDir(groupDir, groupAccountsDir) &&
          _pAccountGroupsDir(accountDir, accountGroupsDir) &&
          _pRemFile(groupAccountsDir + "/" + toURLEncoded(accountName)) &&
          _pRemFile(accountGroupsDir + "/" + toURLEncoded(groupName));
    }
    if (lock) mutex.unlock();
    return r;
}

bool IAuth_FS::groupChangeDescription(const std::string &groupName, const std::string &groupDescription, const std::string & domainName)
{
    bool r = true;
    std::string groupDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pGroupDir(groupName,domainName,groupDir))==true)
        {
            Json::Value groupDetails;
            groupDetails["description"] = groupDescription;
            saveFile_GroupDetails(groupName,domainName, groupDetails);
        }
    }
    
    return r;
}

std::string IAuth_FS::groupDescription(const std::string &groupName, const std::string & domainName)
{
    std::string r;
    std::string groupDir;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((_pGroupDir(groupName,domainName,groupDir))==true)
        {
            Json::Value jDetails = loadFile_GroupDetails(groupName,domainName);
            r = jDetails["description"].asString();
        }
    }
    
    return r;
}

std::list<std::string> IAuth_FS::groupAccounts(const std::string &groupName, const std::string &domainName, bool lock)
{
    std::list<std::string> acccounts;
    std::string groupDir, groupAccountsDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pGroupDir(groupName,domainName,groupDir) && _pGroupAccountsDir(groupDir,groupAccountsDir);
        if (r) acccounts = _pListDir(groupAccountsDir);
    }
    if (lock) mutex.unlock_shared();
    return acccounts;
}


std::list<std::string> IAuth_FS::accountGroups(const std::string &accountName, const std::string & domainName, bool lock)
{
    std::list<std::string> groups;
    std::string accountDir, accountsGroupsDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pAccountDir(accountName, domainName,accountDir) && _pAccountGroupsDir(accountDir,accountsGroupsDir);
        if (r) groups = _pListDir(accountsGroupsDir);
    }
    if (lock) mutex.unlock_shared();
    return groups;
}

bool IAuth_FS::saveFile_GroupDetails(const std::string & group, std::string domainName, const Json::Value & v)
{
    if (domainName == "") domainName = "default";
    return saveJSONFile(workingAuthDir + "/"  + toURLEncoded(domainName) + "/groups/" + toURLEncoded(group) + "/group.details",v);
}

Json::Value IAuth_FS::loadFile_GroupDetails(const std::string &groupName, std::string domainName)
{
    if (domainName == "") domainName = "default";
    return loadJSONFile(workingAuthDir + "/" + toURLEncoded(domainName) + "/groups/" + toURLEncoded(groupName) + "/group.details");
}
