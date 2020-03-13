#include "iauth_sqlite3.h"

bool IAuth_SQLite3::groupAdd(const std::string &groupName, const std::string &groupDescription, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_groups (name,`domain`,description) VALUES(?,?,?);", 3,
                        groupName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        groupDescription.c_str());
    
    return ret;
}

bool IAuth_SQLite3::groupRemove(const std::string &groupName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_groups WHERE name=? AND domain=?;", 2, groupName.c_str(), !domainName.empty()?domainName.c_str():"default");
    return ret;
}

bool IAuth_SQLite3::groupExist(const std::string &groupName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RD lock(mutex);
    std::string xsql= "SELECT name FROM vauth_v1_groups WHERE `domain`=? AND `name`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_text(stmt, 2, groupName.c_str(), groupName.size(), nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW) ret = true;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    
    return ret;
}

bool IAuth_SQLite3::groupAccountAdd(const std::string &groupName, const std::string &accountName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);

    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_groups_accounts (`group_name`,`group_domain`,`account_name`,`account_domain`) VALUES(?,?,?,?);", 4,
                        groupName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        accountName.c_str(),
                        !domainName.empty()?domainName.c_str():"default"
                        );

    
    return ret;
}

bool IAuth_SQLite3::groupAccountRemove(const std::string &groupName, const std::string &accountName, const std::string &domainName, bool lock)
{
    bool ret = false;

    if (lock) mutex.lock();
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_groups_accounts WHERE group_name=? AND account_name=? AND account_domain=?;", 3, groupName.c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");
    if (lock) mutex.unlock();

    return ret;
}

bool IAuth_SQLite3::groupChangeDescription(const std::string &groupName, const std::string &groupDescription, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_groups SET description=? WHERE name=? AND domain=?;", 3, groupDescription.c_str(), groupName.c_str(), !domainName.empty()?domainName.c_str():"default");
    
    return ret;
}

bool IAuth_SQLite3::groupValidateAttribute(const std::string &groupName, const std::string &attribName, std::string domainName, bool lock)
{
    bool ret = false;
    if (lock) mutex.lock_shared();
    std::string xsql= "SELECT group_name FROM vauth_v1_attribs_groups WHERE `attrib_domain`=? AND `attrib_name`=? AND `group_name`=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_text(stmt, 2, attribName.c_str(), attribName.size(), nullptr);
    sqlite3_bind_text(stmt, 3, groupName.c_str(), attribName.size(), nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW) ret = true;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    if (lock) mutex.unlock_shared();
    return ret;
}

std::string IAuth_SQLite3::groupDescription(const std::string &groupName, const std::string &domainName)
{
    std::string ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT description FROM vauth_v1_groups WHERE name=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, groupName.c_str(), groupName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        ret = (const char *)sqlite3_column_text(stmt, 0);
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    
    return ret;
}

std::list<std::string> IAuth_SQLite3::groupsList(const std::string &domainName)
{
    std::list<std::string> ret;
    Lock_Mutex_RD lock(mutex);

    std::string xsql= "SELECT name FROM vauth_v1_groups WHERE `domain`=?;";
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

std::list<std::string> IAuth_SQLite3::groupAttribs(const std::string &groupName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();

    std::string xsql= "SELECT attrib_name FROM vauth_v1_attribs_groups WHERE group_name=? AND group_domain=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, groupName.c_str(), groupName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);
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

std::list<std::string> IAuth_SQLite3::groupAccounts(const std::string &groupName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();

    std::string xsql= "SELECT account_name FROM vauth_v1_groups_accounts WHERE group_name=? AND group_domain=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, groupName.c_str(), groupName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);
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

