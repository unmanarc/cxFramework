#include "iauth_sqlite3.h"

#include <limits>
#include <cx_auth/hlp_auth_strings.h>

bool IAuth_SQLite3::accountAdd(const std::string &accountName, const sPasswordData &passData, const std::string &domainName, const std::string &email, const std::string &accountDescription, const std::string &extraData, time_t expirationDate, bool enabled, bool confirmed, bool superuser)
{
    bool ret = false;

    Lock_Mutex_RW lock(mutex);
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_accounts (name,`domain`,email,description,extraData,superuser) VALUES(?,?,?,?,?,?);", 6,
                        accountName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        email.c_str(),
                        accountDescription.c_str(),
                        extraData.c_str(),
                        std::to_string(superuser?1:0).c_str());
    if (ret)
    {
        ret=_pSQLExecQueryF("INSERT INTO vauth_v1_account_activation (`account`,`domain`,`enabled`,`expiration`,`confirmed`,`confirmationToken`) VALUES(?,?,?,?,?,?);", 6,
                            accountName.c_str(),
                            !domainName.empty()?domainName.c_str():"default",
                            std::to_string(enabled?1:0).c_str(),
                            std::to_string(expirationDate).c_str(),
                            std::to_string(confirmed?1:0).c_str(),
                            genRandomConfirmationToken().c_str()
                            );
    }
    if (ret)
    {
        ret=_pSQLExecQueryF("INSERT INTO vauth_v1_account_passwords (`index`,`account`,`domain`,`hash`,`expiration`,`mode`,`salt`,`forcedExpiration`,`steps`) VALUES(0,?,?,?,?,?,?,?,?);", 8,
                            accountName.c_str(),
                            !domainName.empty()?domainName.c_str():"default",
                            passData.hash.c_str(),
                            std::to_string(passData.expiration).c_str(),
                            std::to_string(passData.passwordMode).c_str(),
                            strToHex(passData.ssalt,4).c_str(),
                            std::to_string(passData.forceExpiration?1:0).c_str(),
                            std::to_string(passData.gAuthSteps).c_str()
                            );
    }


    return ret;
}

std::string IAuth_SQLite3::accountConfirmationToken(const std::string &accountName, const std::string &domainName)
{
    std::string ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT confirmationToken FROM vauth_v1_account_activation WHERE account=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

bool IAuth_SQLite3::accountRemove(const std::string &accountName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("DELETE FROM vauth_v1_accounts WHERE name=? AND domain=?;", 2, accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::accountDisable(const std::string &accountName, const std::string &domainName, bool disabled)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_account_activation SET enabled=? WHERE name=? AND domain=?;", 3, disabled?"0":"1", accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::accountConfirm(const std::string &accountName, const std::string &confirmationToken, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);

    std::string xsql = "SELECT confirmationToken FROM vauth_v1_account_activation WHERE account=? AND `domain`=? LIMIT 1";
    std::string dbConfirmationToken, curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        dbConfirmationToken = (const char *)sqlite3_column_text(stmt, 0);
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    if (!dbConfirmationToken.empty() && confirmationToken==dbConfirmationToken)
    {
        ret = _pSQLExecQueryF("UPDATE vauth_v1_account_activation SET confirmed=? WHERE account=? AND domain=?;", 3, "1", accountName.c_str(), !domainName.empty()?domainName.c_str():"default");
    }


    return ret;
}

bool IAuth_SQLite3::accountChangePassword(const std::string &accountName, const sPasswordData &passwordData, uint32_t passIndex, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    _pSQLExecQueryF("DELETE FROM vauth_v1_account_passwords WHERE account=? and domain=? and `index`=?",3,
                    accountName.c_str(),
                    !domainName.empty()?domainName.c_str():"default",
                    std::to_string(passIndex).c_str()
                    );
    ret=_pSQLExecQueryF("INSERT INTO vauth_v1_account_passwords (`index`,`account`,`domain`,`hash`,`expiration`,`mode`,`salt`,`forcedExpiration`,`steps`) VALUES(?,?,?,?,?,?,?,?,?);", 9,
                        std::to_string(passIndex).c_str(),
                        accountName.c_str(),
                        !domainName.empty()?domainName.c_str():"default",
                        passwordData.hash.c_str(),
                        std::to_string(passwordData.expiration).c_str(),
                        std::to_string(passwordData.passwordMode).c_str(),
                        strToHex(passwordData.ssalt,4).c_str(),
                        std::to_string(passwordData.forceExpiration?1:0).c_str(),
                        std::to_string(passwordData.gAuthSteps).c_str()
                        );

    return ret;
}

bool IAuth_SQLite3::accountChangeDescription(const std::string &accountName, const std::string &description, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_accounts SET description=? WHERE name=? AND domain=?;", 3, description.c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::accountChangeEmail(const std::string &accountName, const std::string &email, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_accounts SET email=? WHERE name=? AND domain=?;", 3, email.c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::accountChangeExtraData(const std::string &accountName, const std::string &extraData, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_accounts SET extraData=? WHERE name=? AND domain=?;", 3, extraData.c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::accountChangeExpiration(const std::string &accountName, const std::string &domainName, time_t expiration)
{
    bool ret = false;
    Lock_Mutex_RW lock(mutex);
    ret = _pSQLExecQueryF("UPDATE vauth_v1_account_activation SET expiration=? WHERE account=? AND domain=?;", 3, std::to_string(expiration).c_str(), accountName.c_str(), !domainName.empty()?domainName.c_str():"default");

    return ret;
}

bool IAuth_SQLite3::isAccountDisabled(const std::string &accountName, const std::string &domainName)
{
    bool ret = true;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT enabled FROM vauth_v1_account_activation WHERE account=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        ret = sqlite3_column_int(stmt, 0)==0?true:false;
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return ret;
}

bool IAuth_SQLite3::isAccountConfirmed(const std::string &accountName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT confirmed FROM vauth_v1_account_activation WHERE account=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        ret = sqlite3_column_int(stmt, 0)==1?true:false;
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return ret;
}

bool IAuth_SQLite3::isAccountSuperUser(const std::string &accountName, const std::string &domainName)
{
    bool ret = false;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT superuser FROM vauth_v1_accounts WHERE name=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        ret = sqlite3_column_int(stmt, 0)==1?true:false;
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return ret;
}

std::string IAuth_SQLite3::accountDescription(const std::string &accountName, const std::string &domainName)
{
    std::string ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT description FROM vauth_v1_accounts WHERE name=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

std::string IAuth_SQLite3::accountEmail(const std::string &accountName, const std::string &domainName)
{
    std::string ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT email FROM vauth_v1_accounts WHERE name=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

std::string IAuth_SQLite3::accountExtraData(const std::string &accountName, const std::string &domainName)
{
    std::string ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT extraData FROM vauth_v1_accounts WHERE name=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

time_t IAuth_SQLite3::accountExpirationDate(const std::string &accountName, const std::string &domainName)
{
    time_t ret = std::numeric_limits<time_t>::max();
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT expiration FROM vauth_v1_account_activation WHERE account=? AND `domain`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
        ret = (time_t) sqlite3_column_int(stmt, 0);
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return ret;
}

std::list<std::string> IAuth_SQLite3::accountsList(const std::string &domainName)
{
    std::list<std::string> ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql= "SELECT name FROM vauth_v1_accounts WHERE `domain`=?;";
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

std::list<std::string> IAuth_SQLite3::accountGroups(const std::string &accountName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();

    std::string xsql= "SELECT group_name FROM vauth_v1_groups_accounts WHERE  account_name=? AND account_domain=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

std::list<std::string> IAuth_SQLite3::accountDirectAttribs(const std::string &accountName, const std::string &domainName, bool lock)
{
    std::list<std::string> ret;
    if (lock) mutex.lock_shared();

    std::string xsql= "SELECT attrib_name FROM vauth_v1_attribs_accounts WHERE account_name=? AND account_domain=?;";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
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

sPasswordData IAuth_SQLite3::retrievePassword(const std::string &accountName, const std::string &domainName, uint32_t passIndex, bool *found)
{
    sPasswordData ret;
    Lock_Mutex_RD lock(mutex);
    std::string xsql = "SELECT steps,forcedExpiration,mode,expiration,salt,hash FROM vauth_v1_account_passwords WHERE `account`=? AND `domain`=? AND `index`=? LIMIT 1";
    std::string curDomain = !domainName.empty()?domainName.c_str():"default";

    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
    sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.size(), nullptr);
    sqlite3_bind_text(stmt, 2, curDomain.c_str(), curDomain.size(), nullptr);
    sqlite3_bind_int(stmt, 3, (int)passIndex);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
    {
        *found = true;
        ret.gAuthSteps = (uint32_t)sqlite3_column_int(stmt, 0);
        ret.forceExpiration = sqlite3_column_int(stmt, 1)==0?false:true;
        ret.passwordMode = (PasswordModes)sqlite3_column_int(stmt, 2);
        ret.expiration = (time_t)sqlite3_column_int64(stmt, 3);
        std::string salt = (const char *)sqlite3_column_text(stmt, 4);
        strFromHex(salt,ret.ssalt,4);
        ret.hash = (const char *)sqlite3_column_text(stmt, 5);
    }
    else
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return ret;
}
