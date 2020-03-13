#include "iauth_sqlite3.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

IAuth_SQLite3::IAuth_SQLite3(const std::string &appName, const std::string &filePath)
{
    this->filePath = filePath;
    this->appName = appName;
//    if (initScheme()) initAccounts();
}


bool IAuth_SQLite3::initScheme()
{
    bool ret = true;
    // create the scheme to handle the accounts.
    std::string etcAppDir = "/etc/" + appName;

    if (getuid()==0 && access(etcAppDir.c_str(), W_OK))
    {
        // We are root, but directory does not exist.
        // then, create it.
        mkdir(etcAppDir.c_str(), 0750);
    }

    std::string etcAppAuthFile;

    if (!filePath.empty())
        etcAppAuthFile = filePath;
    else
    {
        // Try to use /etc/appname dir.
        if (!access(etcAppDir.c_str(), W_OK))
        {
            // Use /etc/appname dir.
            ret = true;
            etcAppAuthFile = "/etc/" + appName + "/auth.db";
        }
        else
        {
            // Use $HOME/.config/appName dir.
            ret = true;
            struct passwd *pw = getpwuid(getuid());
            const char *homedir = pw->pw_dir;

            std::string homeConfigDir = std::string(homedir) + "/.config";
            std::string homeAppConfigDir = std::string(homedir) + "/.config/" + appName;

            // Can't use /etc/appname, use homedir.
            if (access(homeConfigDir.c_str(), W_OK))             ret=!mkdir(etcAppDir.c_str(), 0750);
            if (ret && access(homeAppConfigDir.c_str(), W_OK))   ret=!mkdir(homeAppConfigDir.c_str(), 0750);

            if (ret) etcAppAuthFile = homeAppConfigDir + "/auth.db";
        }
    }

    if (ret)
    {
        int rc;
        rc = sqlite3_open(etcAppAuthFile.c_str(), &ppDb);
        if (rc)
        {
            fprintf(stderr, " SQL Error: Unable to create/open log file (%s) - %s\n", etcAppAuthFile.c_str(), sqlite3_errmsg(ppDb));
            ret = false;
        }
        else
        {
            _pSQLExecQuery("PRAGMA foreign_keys = ON;");

            if (!_pSQLTableExist("vauth_v1_attribs_accounts"))
            {
                _pSQLExecQuery("CREATE TABLE \"vauth_v1_domains\" (\n"
                               "       `name`  VARCHAR(256),\n"
                               "       PRIMARY KEY(`name`)\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_attribs` (\n"
                               "       `name`  VARCHAR(256) NOT NULL,\n"
                               "       `domain`        VARCHAR(256),\n"
                               "       `description`   VARCHAR(4096),\n"
                               "       PRIMARY KEY(`name`,`domain`),\n"
                               "       FOREIGN KEY(`domain`) REFERENCES vauth_v1_domains(`name`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_accounts` (\n"
                               "       `name`  VARCHAR(256) NOT NULL,\n"
                               "       `domain`        VARCHAR(256) NOT NULL,\n"
                               "       `email` VARCHAR(1024),\n"
                               "       `description`   VARCHAR(4096),\n"
                               "       `extraData`     VARCHAR(4096),\n"
                               "       `superuser`     BOOLEAN,\n"
                               "       PRIMARY KEY(`name`,`domain`),\n"
                               "       FOREIGN KEY(`domain`) REFERENCES vauth_v1_domains(`name`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_account_activation` (\n"
                               "       `account`       VARCHAR(256) NOT NULL,\n"
                               "       `domain`        VARCHAR(256) NOT NULL,\n"
                               "       `enabled`       BOOLEAN,\n"
                               "       `expiration`    DATETIME,\n"
                               "       `confirmed`     BOOLEAN,\n"
                               "       `confirmationToken`       VARCHAR(256) NOT NULL,\n"
                               "       PRIMARY KEY(`account`,`domain`),\n"
                               "       FOREIGN KEY(`account`,`domain`) REFERENCES vauth_v1_accounts(`name`,`domain`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_account_passwords` (\n"
                               "       `index` INTEGER NOT NULL,\n"
                               "       `account`       VARCHAR(256) NOT NULL,\n"
                               "       `domain`        VARCHAR(256) NOT NULL,\n"
                               "       `hash`  VARCHAR(256),\n"
                               "       `expiration`    DATETIME DEFAULT NULL,\n"
                               "       `mode`  INTEGER DEFAULT 0,\n"
                               "       `salt`  VARCHAR(256),\n"
                               "       `forcedExpiration`      BOOLEAN DEFAULT 0,\n"
                               "       `steps` INTEGER DEFAULT 0,\n"
                               "       PRIMARY KEY(`index`,`account`,`domain`),\n"
                               "       FOREIGN KEY(`account`,`domain`) REFERENCES vauth_v1_accounts(`name`,`domain`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE \"vauth_v1_groups\" (\n"
                               "       `name`  VARCHAR(256) NOT NULL,\n"
                               "       `domain`        VARCHAR(256) NOT NULL,\n"
                               "       `description`   VARCHAR(4096),\n"
                               "       PRIMARY KEY(`name`,`domain`),\n"
                               "       FOREIGN KEY(`domain`) REFERENCES vauth_v1_domains(`name`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_groups_accounts` (\n"
                               "       `group_name`    VARCHAR(256) NOT NULL,\n"
                               "       `group_domain`  VARCHAR(256) NOT NULL,\n"
                               "       `account_name`  VARCHAR(256) NOT NULL,\n"
                               "       `account_domain`        VARCHAR(256) NOT NULL,\n"
                               "       FOREIGN KEY(`group_name`, `group_domain`) REFERENCES vauth_v1_groups(`name`,`domain`) ON DELETE CASCADE,\n"
                               "       FOREIGN KEY(`account_name`,`account_domain`) REFERENCES vauth_v1_accounts(`name`,`domain`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_attribs_groups` (\n"
                               "       `attrib_name`   VARCHAR(256) NOT NULL,\n"
                               "       `attrib_domain` VARCHAR(256) NOT NULL,\n"
                               "       `group_name`    VARCHAR(256) NOT NULL,\n"
                               "       `group_domain`  VARCHAR(256) NOT NULL,\n"
                               "       FOREIGN KEY(`attrib_name`, `attrib_domain`) REFERENCES vauth_v1_attribs(`name`,`domain`) ON DELETE CASCADE,\n"
                               "       FOREIGN KEY(`group_name`,`group_domain`) REFERENCES vauth_v1_groups(`name`,`domain`) ON DELETE CASCADE\n"
                               ");\n");
                _pSQLExecQuery("CREATE TABLE `vauth_v1_attribs_accounts` (\n"
                               "       `attrib_name`    VARCHAR(256) NOT NULL,\n"
                               "       `attrib_domain`  VARCHAR(256) NOT NULL,\n"
                               "       `account_name`   VARCHAR(256) NOT NULL,\n"
                               "       `account_domain` VARCHAR(256) NOT NULL,\n"
                               "       FOREIGN KEY(`attrib_name`, `attrib_domain`) REFERENCES vauth_v1_attribs(`name`,`domain`) ON DELETE CASCADE,\n"
                               "       FOREIGN KEY(`account_name`,`account_domain`) REFERENCES vauth_v1_accounts(`name`,`domain`) ON DELETE CASCADE\n"
                               ");\n");

                _pSQLExecQuery("CREATE UNIQUE INDEX `idx_groups_accounts` ON `vauth_v1_groups_accounts` (`group_name` ,`group_domain` ,`account_name` ,`account_domain` );\n");
                _pSQLExecQuery("CREATE UNIQUE INDEX `idx_attribs_groups` ON `vauth_v1_attribs_groups` (`attrib_name` ,`attrib_domain` ,`group_name` ,`group_domain` );\n");
                _pSQLExecQuery("CREATE UNIQUE INDEX `idx_attribs_accounts` ON `vauth_v1_attribs_accounts` (`attrib_name` ,`attrib_domain` ,`account_name` ,`account_domain` );\n");
            }
            ret = true;
        }
    }

    return ret;
}

bool IAuth_SQLite3::_pSQLTableExist(const std::string &table)
{
    bool ret;
    std::string xsql = "select sql from sqlite_master where tbl_name=?;";
    sqlite3_stmt * stmt = nullptr;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, table.c_str(), table.size(), nullptr);
    int s = sqlite3_step(stmt);
    ret = (s == SQLITE_ROW ? true : false);
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    return ret;
}

bool IAuth_SQLite3::_pSQLExecQuery(const std::string &query)
{
    const char *tail;
    sqlite3_stmt *stmt = nullptr;

    sqlite3_prepare_v2(ppDb, query.c_str(), query.length(), &stmt, &tail);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
        return false;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    return true;
}

bool IAuth_SQLite3::_pSQLExecQueryF(const std::string &query, int _va_size, ...)
{
    const char *tail;
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare_v2(ppDb, query.c_str(), query.length(), &stmt, &tail);

    va_list args;
    va_start(args, _va_size);

    for (int i = 0; i < _va_size; i++)
    {
        const char *val_to_bind = va_arg(args, const char *);
        sqlite3_bind_text(stmt, i + 1, val_to_bind, strlen(val_to_bind), SQLITE_TRANSIENT);
    }

    va_end(args);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        //fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
        return false;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return true;
}
