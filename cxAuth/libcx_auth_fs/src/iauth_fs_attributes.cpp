#include "iauth_fs.h"

#include <cx_auth/hlp_auth_strings.h>
#include "iauth_fs_helper_jsonfileaccess.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>

bool IAuth_FS::_pAttribDir(const std::string &attribName, std::string domainName, std::string &attribDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    if (domainName == "") domainName = "default";
    attribDirOut = workingAuthDir + "/"  + toURLEncoded(domainName) + "/attribs/" + toURLEncoded(attribName);
    return !access(attribDirOut.c_str(),W_OK);
}

bool IAuth_FS::_pAttribDirCreate(const std::string &attribName, std::string domainName, std::string &attribsDirOut)
{
    std::string domainDir;
    if (!_pDomainDir(domainName, domainDir)) return false;
    std::string attribsDir = domainDir + "/attribs/" + toURLEncoded(attribName);
    if (access(attribsDir.c_str(), W_OK) && mkdir(attribsDir.c_str(),0750)) return false;
    attribsDirOut = attribsDir;
    return true;
}

bool IAuth_FS::_pAttribAccountsDir(const std::string &attribDir, std::string &attribAccountsOut)
{
    attribAccountsOut = attribDir + "/accounts";
    return !access(attribAccountsOut.c_str(), W_OK);
}

bool IAuth_FS::_pAttribGroupsDir(const std::string &attribDir, std::string &attribGroupsDirOut)
{
    attribGroupsDirOut = attribDir + "/groups";
    return !access(attribGroupsDirOut.c_str(), W_OK);
}

bool IAuth_FS::_pAttribsDir(std::string domainName, std::string &accountsDirOut)
{
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    if (domainName == "") domainName = "default";
    accountsDirOut = workingAuthDir + "/"  + toURLEncoded(domainName) + "/attribs";
    return !access(accountsDirOut.c_str(),W_OK);
}

std::list<std::string> IAuth_FS::attribsList(const std::string & domainName)
{
    std::list<std::string> attribs;
    Lock_Mutex_RD lock(mutex);
    std::string accountsDir;
    if (_pAttribsDir(domainName,accountsDir))
    {
        attribs = _pListDir(accountsDir);
    }
    
    return attribs;
}

bool IAuth_FS::attribAdd(const std::string &attribName, const std::string &attribDescription, std::string domainName)
{
    std::string attribDir, attribDirAccounts, attribDirGroups;
    bool r=false;

    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        r=!_pAttribDir(attribName,domainName,attribDir);
        if (r) r=!mkdir(attribDir.c_str(), 0750);
        if (r && (r=_pAttribAccountsDir(attribDir, attribDirAccounts))==false)
            r=!mkdir(attribDirAccounts.c_str(), 0750);
        if (r && (r=_pAttribGroupsDir(attribDir, attribDirGroups))==false)
            r=!mkdir(attribDirGroups.c_str(), 0750);

        if (r)
        {
            // Dir created here, now fill the data.
            Json::Value attribDetails;
            attribDetails["description"] = attribDescription;
            saveFile_AttribDetails(attribName,domainName, attribDetails);
        }
    }
    
    return r;
}

bool IAuth_FS::attribRemove(const std::string &attribName, const std::string & domainName)
{
    std::string attribDir;
    bool r = false;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAttribDir(attribName,domainName,attribDir))==true)
        {
            // Remove from accounts association.
            for (const std::string & accountName : attribAccounts(attribName, domainName,false))
                attribAccountRemove(attribName,domainName,accountName,false);

            // Remove from group association.
            for (const std::string & groupName : attribGroups(attribName, domainName, false))
                attribGroupRemove(attribName,domainName,groupName,false);

            // Remove attrib dir.
            boost::filesystem::remove_all(attribDir);
        }
    }
    
    return r;
}

bool IAuth_FS::attribGroupAdd(const std::string & attribName, const std::string & groupName, const std::string &domainName)
{
    std::string groupDir, groupAttribsDir;
    std::string attribDir, attribGroupsDir;
    bool r=false;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        r =     _pAttribDir(attribName,domainName,attribDir) &&
                _pGroupDir(groupName,domainName,groupDir) &&
                _pGroupAttribsDir(groupDir,groupAttribsDir) &&
                _pAttribGroupsDir(attribDir,attribGroupsDir) &&
                _pTouchFile(attribGroupsDir + "/" + toURLEncoded(groupName)) &&
                _pTouchFile(groupAttribsDir + "/" + toURLEncoded(attribName));
    }
    
    return r;
}

bool IAuth_FS::attribGroupRemove(const std::string &attribName, const std::string &groupName,const std::string & domainName, bool lock)
{
    std::string groupDir, groupAttribsDir;
    std::string attribDir, attribGroupsDir;
    bool r=false;
    if (lock) mutex.lock();
    if (!workingAuthDir.empty())
    {
        r =     _pAttribDir(attribName,domainName,attribDir) &&
                _pGroupDir(groupName,domainName,groupDir) &&
                _pGroupAttribsDir(groupDir,groupAttribsDir) &&
                _pAttribGroupsDir(attribDir,attribGroupsDir) &&
                _pRemFile(attribGroupsDir + "/" + toURLEncoded(groupName)) &&
                _pRemFile(groupAttribsDir + "/" + toURLEncoded(attribName));
    }
    if (lock) mutex.unlock();
    return r;
}

bool IAuth_FS::attribAccountAdd(const std::string &attribName, const std::string &accountName,const std::string & domainName)
{
    std::string accountDir, accountAttribsDir;
    std::string attribDir, attribAccountsDir;

    bool r = false;
    Lock_Mutex_RW lock(mutex);

    if (!workingAuthDir.empty())
    {
        r =     _pAttribDir(attribName,domainName,attribDir) &&
                _pAccountDir(accountName,domainName,accountDir) &&
                _pAccountAttribsDir(accountDir,accountAttribsDir) &&
                _pAttribAccountsDir(attribDir,attribAccountsDir) &&
                _pTouchFile(attribAccountsDir + "/" + toURLEncoded(accountName)) &&
                _pTouchFile(accountAttribsDir + "/" + toURLEncoded(attribName));
    }

    
    return r;
}

bool IAuth_FS::attribAccountRemove(const std::string &attribName, const std::string &accountName,const std::string & domainName, bool lock)
{
    std::string accountDir, accountAttribsDir;
    std::string attribDir, attribAccountsDir;

    bool r = false;
    if (lock) mutex.lock();

    if (!workingAuthDir.empty())
    {
        r =     _pAttribDir(attribName,domainName,attribDir) &&
                _pAccountDir(accountName,domainName,accountDir) &&
                _pAccountAttribsDir(accountDir,accountAttribsDir) &&
                _pAttribAccountsDir(attribDir,attribAccountsDir) &&
                _pRemFile(attribAccountsDir + "/" + toURLEncoded(accountName)) &&
                _pRemFile(accountAttribsDir + "/" + toURLEncoded(attribName));
    }

    if (lock) mutex.unlock();
    return r;
}

bool IAuth_FS::attribChangeDescription(const std::string &attribName, const std::string &attribDescription,const std::string & domainName)
{
    bool r = false;
    std::string attribDir;
    Lock_Mutex_RW lock(mutex);
    if (!workingAuthDir.empty())
    {
        if ((r=_pAttribDir(attribName,domainName,attribDir))==true)
        {
            Json::Value attribDetails;
            attribDetails["description"] = attribDescription;
            saveFile_AttribDetails(attribName, domainName, attribDetails);
        }
    }
    
    return r;
}

std::list<std::string> IAuth_FS::groupAttribs(const std::string &groupName,const std::string & domainName, bool lock)
{
    std::list<std::string> attribs;
    std::string groupDir, groupAttribsDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pGroupDir(groupName,domainName,groupDir) && _pGroupAttribsDir(groupDir,groupAttribsDir);
        if (r) attribs = _pListDir(groupAttribsDir);
    }
    if (lock) mutex.unlock_shared();
    return attribs;
}

std::list<std::string> IAuth_FS::attribGroups(const std::string &attribName, const std::string & domainName, bool lock)
{
    std::list<std::string> groups;
    std::string attribDir, attribGroupsDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pAttribDir(attribName,domainName,attribDir) && _pAttribGroupsDir(attribDir,attribGroupsDir);
        if (r) groups = _pListDir(attribGroupsDir);
    }
    if (lock) mutex.unlock_shared();
    return groups;
}

std::list<std::string> IAuth_FS::attribAccounts(const std::string &attribName, const std::string &domainName, bool lock)
{
    std::list<std::string> accounts;
    std::string attribDir, attribAccountsDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pAttribDir(attribName,domainName,attribDir) && _pAttribAccountsDir(attribDir,attribAccountsDir);
        if (r) accounts = _pListDir(attribAccountsDir);
    }
    if (lock) mutex.unlock_shared();
    return accounts;
}

std::list<std::string> IAuth_FS::accountDirectAttribs(const std::string &accountName, const std::string & domainName, bool lock)
{
    std::list<std::string> attribs;
    std::string accountsDir, accountsAttribDir;
    bool r = false;
    if (lock) mutex.lock_shared();
    if (!workingAuthDir.empty())
    {
        r = _pAccountDir(accountName, domainName,accountsDir) && _pAccountAttribsDir(accountsDir,accountsAttribDir);
        if (r) attribs = _pListDir(accountsAttribDir);
    }
    if (lock) mutex.unlock_shared();
    return attribs;
}


bool IAuth_FS::accountValidateDirectAttribute(const std::string &accountName, const std::string &attribName, std::string domainName)
{
    if (domainName == "") domainName = "default";
    Lock_Mutex_RD lock(mutex);
    std::string attribFile = workingAuthDir + "/" + toURLEncoded(domainName) + "/attribs/" + toURLEncoded(attribName) + "/accounts/" + toURLEncoded(accountName);
    if (!access(attribFile.c_str(),R_OK))
    {
        
        return true;
    }
    
    return false;
}

bool IAuth_FS::groupValidateAttribute(const std::string &groupName, const std::string &attribName, std::string domainName, bool lock)
{
    if (domainName == "") domainName = "default";
    if (lock) mutex.lock_shared();
    std::string attribFile = workingAuthDir + "/" + toURLEncoded(domainName) + "/attribs/" + toURLEncoded(attribName) + "/groups/" + toURLEncoded(groupName);
    bool r = !access(attribFile.c_str(),R_OK);
    if (lock) mutex.unlock_shared();
    return r;
}

bool IAuth_FS::saveFile_AttribDetails(const std::string & attribName, std::string domainName, const Json::Value & v)
{
    if (domainName == "") domainName = "default";
    return saveJSONFile(workingAuthDir + "/" + toURLEncoded(domainName) + "/attribs/" + toURLEncoded(attribName) + "/attribs.details",v);
}

Json::Value IAuth_FS::loadFile_AttribDetails(const std::string &attribName, std::string domainName)
{
    if (domainName == "") domainName = "default";
    return loadJSONFile(workingAuthDir + "/" + toURLEncoded(domainName) + "/attribs/" + toURLEncoded(attribName) + "/attribs.details");
}
