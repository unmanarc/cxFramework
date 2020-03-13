#include "iauth_sqlite3.h"



bool IAuth_SQLite3::domainAdd(const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_domains (name) VALUES(?);",1, !domainName.empty()?domainName.c_str():"default");
    
    return ret;
}

bool IAuth_SQLite3::domainRemove(const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("DELETE FROM vauth_v1_domains WHERE name=?;",1, !domainName.empty()?domainName.c_str():"default");
    
    return ret;
}

std::list<std::string> IAuth_SQLite3::domainList()
{
    std::list<std::string> ret;
    Lock_Mutex_RD lock(mutex);
    
    return ret;
}

