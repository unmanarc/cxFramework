#include "iauth_fs.h"
#include <unistd.h>
#include <cx_auth/hlp_auth_strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>

bool IAuth_FS::_pDomainDir(std::string domainName, std::string &domainDirOut)
{
    if (domainName == "") domainName = "default";
    if (access(workingAuthDir.c_str(), W_OK)) return false;
    std::string domainDir  = workingAuthDir + "/"  + toURLEncoded(domainName);
    if (access(domainDir.c_str(), W_OK)) return false;
    domainDirOut = domainDir;
    return true;
}

bool IAuth_FS::_pDomainDirCreate(std::string domainName, std::string &domainDirOut)
{
    if (domainName == "") domainName = "default";
    if (access(workingAuthDir.c_str(), W_OK)) return false;

    std::string domainDir  = workingAuthDir + "/"  + toURLEncoded(domainName);
    std::string domainAccountsDir  = domainDir + "/accounts";
    std::string domainGroupsDir  = domainDir + "/groups";
    std::string domainAttribsDir  = domainDir + "/attribs";

    if (access(domainDir.c_str(), W_OK) && mkdir(domainDir.c_str(),0750)) return false;
    if (access(domainAccountsDir.c_str(), W_OK) && mkdir(domainAccountsDir.c_str(),0750)) return false;
    if (access(domainGroupsDir.c_str(), W_OK) && mkdir(domainGroupsDir.c_str(),0750)) return false;
    if (access(domainAttribsDir.c_str(), W_OK) && mkdir(domainAttribsDir.c_str(),0750)) return false;

    domainDirOut = domainDir;
    return true;
}

std::list<std::string> IAuth_FS::domainList()
{
    std::list<std::string> domains;
    Lock_Mutex_RD lock(mutex);
    if (!workingAuthDir.empty() && !access(workingAuthDir.c_str(), W_OK))
    {
        domains = _pListDir(workingAuthDir);
    }
    
    return domains;
}

bool IAuth_FS::domainAdd(const std::string &domainName)
{
    std::string domainDir;
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    if (!_pDomainDir(domainName,domainDir))
        ret = _pDomainDirCreate(domainName,domainDir);
    return ret;
}

bool IAuth_FS::domainRemove(const std::string &domainName)
{
    std::string domainDir;
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    if (_pDomainDir(domainName,domainDir))
    {
        boost::filesystem::remove_all(domainDir);
        ret = true;
    }
    return ret;
}
