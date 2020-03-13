#include "iauth_fs.h"

//#include <boost/filesystem.hpp>

#include <fstream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pwd.h>

#include <sys/types.h>
#include <dirent.h>

#include <cx_auth/hlp_auth_strings.h>

IAuth_FS::IAuth_FS(const std::string &appName)
{
    this->appName = appName;
//    if (initScheme()) initAccounts();
}

bool IAuth_FS::initScheme()
{
    bool r = true;
    // create the scheme to handle the accounts.
    std::string etcAppDir = "/etc/" + appName;

    if (getuid()==0 && access(etcAppDir.c_str(), W_OK))
    {
        // We are root, but directory does not exist.
        // then, create it.
        mkdir(etcAppDir.c_str(), 0750);
    }

    // Try to use /etc/appname dir.
    if (!access(etcAppDir.c_str(), W_OK))
    {
        std::string etcVAuthDir = etcAppDir + "/auth";
        if (access(etcVAuthDir.c_str(), W_OK)) r=!mkdir(etcVAuthDir.c_str(), 0750);
        if (r)
        {
            // Now we are ready to handle this.
            workingAuthDir = etcVAuthDir;
            return true;
        }
    }

    // If not. try to use .config
    r = true;
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    std::string homeConfigDir = std::string(homedir) + "/.config";
    std::string homeAppConfigDir = std::string(homedir) + "/.config/" + appName;
    std::string homeVAuthDir = homeAppConfigDir + "/auth";

    // Can't use /etc/appname, use homedir.
    if (access(homeConfigDir.c_str(), W_OK))             r=!mkdir(etcAppDir.c_str(), 0750);
    if (r && access(homeAppConfigDir.c_str(), W_OK))     r=!mkdir(homeAppConfigDir.c_str(), 0750);
    if (r && access(homeVAuthDir.c_str(), W_OK))         r=!mkdir(homeVAuthDir.c_str(), 0750);

    if (r)
    {
        // Now we are ready to handle this.
        workingAuthDir = homeVAuthDir;
    }

    return r;
}

bool IAuth_FS::_pTouchFile(const std::string &fileName, const std::string &value)
{
    std::ofstream ofstr;
    ofstr.open(fileName);
    if (ofstr.fail()) return false;
    ofstr << "1";
    ofstr.close();
    return true;
}

bool IAuth_FS::_pRemFile(const std::string &fileName)
{
    return !unlink(fileName.c_str());
}

std::list<std::string> IAuth_FS::_pListDir(const std::string &dirPath)
{
    std::list<std::string> x;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != nullptr)
    {
        while ((ent = readdir (dir)) != nullptr)
        {
            if (ent->d_type == DT_REG)
                x.push_back(fromURLEncoded(ent->d_name));
        }
        closedir (dir);
    }
    return x;
}

