#include "iauth_sqlite3.h"


bool IAuth_SQLite3::attribAdd(const std::string &attribName, const std::string &attribDescription, std::string domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_attribs (name,`domain`,description) VALUES(?,?,?);", 3,
                        attribName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        attribDescription.c_str());
    
    return ret;
}

bool IAuth_SQLite3::attribRemove(const std::string &attribName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_attribs WHERE name=? AND domain=?;", 2, attribName.c_str(), !domainName.empty()?domainName.c_str():"default");
    
    return ret;
}

bool IAuth_SQLite3::attribGroupAdd(const std::string &attribName, const std::string &groupName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_attribs_groups (`attrib_name`,`attrib_domain`,`group_name`,`group_domain`) VALUES(?,?,?,?);", 4,
                        attribName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        groupName.c_str(),
                        !domainName.empty()?domainName.c_str():"default"
                        );
    
    return ret;
}

bool IAuth_SQLite3::attribGroupRemove(const std::string &attribName, const std::string &groupName, const std::string &domainName, bool lock)
{
    bool ret = false;
    if (lock) mutex.lock();
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_attribs_groups WHERE attrib_name=? AND group_name=? AND group_domain=?;", 3, attribName.c_str(), groupName.c_str(), !domainName.empty()?domainName.c_str():"default");
    if (lock) mutex.unlock();
    return ret;
}

bool IAuth_SQLite3::attribAccountAdd(const std::string &attribName, const std::string &accountName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_attribs_accounts (`attrib_name`,`attrib_domain`,`account_name`,`account_domain`) VALUES(?,?,?,?);", 4,
                        attribName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        accountName.c_str(),
                        !domainName.empty()?domainName.c_str():"default"
                        );
    
    return ret;
}

bool IAuth_SQLite3::attribAccountRemove(const std::string &attribName, const std::string &accountName, const std::string &domainName, bool lock)
{
    bool ret = false;
    if (lock) mutex.lock();
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_attribs_accounts WHERE attrib_name=? AND account_name=? AND account_domain=?;", 3, attribName.c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");
    if (lock) mutex.unlock();
    return ret;
}

bool IAuth_SQLite3::attribChangeDescription(const std::string &attribName, const std::string &attribDescription, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_attribs SET description=? WHERE name=? AND domain=?;", 3, attribDescription.c_str(), attribName.c_str(), !domainName.empty()?domainName.c_str():"default");
    
    return ret;
}

std::list<std::string> IAuth_SQLite3::attribsList(const std::string &domainName)
{
    std::list<std::string> ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql= "SELECT name FROM vauth_v1_attribs WHERE `domain`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    for (;;)
    {
        int s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            ret.push_back((const char *)sqlite3_column_text(stmt, 0));
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
        else
        {
            //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    
    return ret;
}

std::list<std::string> IAuth_SQLite3::attribGroups(const std::string &attribName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();
    std::string xsql= "SELECT group_name FROM vauth_v1_attribs_groups WHERE `attrib_domain`=? AND `attrib_name`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_text(stmt, 2, attribName.c_str(), attribName.size(), nullptr);
    for (;;)
    {
        int s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            ret.push_back((const char *)sqlite3_column_text(stmt, 0));
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
        else
        {
            //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    if (lock) mutex.unlock_shared();
    return ret;
}

std::list<std::string> IAuth_SQLite3::attribAccounts(const std::string &attribName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();
    std::string xsql= "SELECT account_name FROM vauth_v1_attribs_accounts WHERE `attrib_domain`=? AND `attrib_name`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_text(stmt, 2, attribName.c_str(), attribName.size(), nullptr);
    for (;;)
    {
        int s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            ret.push_back((const char *)sqlite3_column_text(stmt, 0));
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
        else
        {
            //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    if (lock) mutex.unlock_shared();
    return ret;
}

bool IAuth_SQLite3::accountValidateDirectAttribute(const std::string &accountName, const std::string &attribName, std::string domainName)
{
    bool ret=false;
    Lock_Mutex_RD lock(mutex);
    std::string xsql= "SELECT account_name FROM vauth_v1_attribs_accounts WHERE `attrib_domain`=? AND `attrib_name`=? AND `account_name`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_text(stmt, 2, attribName.c_str(), attribName.size(), nullptr);
    sqlite3_bind_text(stmt, 3, accountName.c_str(), attribName.size(), nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW) ret = true;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    
    return ret;
}
