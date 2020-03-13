#include "applog.h"
#ifdef _WIN32
#include <shlobj.h>
#else
#include <pwd.h>
#include <syslog.h>
#endif


#include <arpa/inet.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

AppLog::AppLog(const std::string & _appName, const std::string & _logName, unsigned int _logMode)
{
    appName = _appName;
    logName = _logName;
    logMode = _logMode;

    // variable initialization.
    standardLogSeparator = " ";
    usingPrintDate = true;
    usingAttributeName = true;
    printEmptyFields = false;
    usingColors = true;
    debug = false;

    moduleAlignSize = 13;
    userAlignSize = 13;

#ifdef _WIN32
    char szPath[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL,CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,NULL,0,szPath)))
    {
        appLogDir = string(szPath) + "\\" + _appName;
    }
    else
    {
        appLogDir = "c:\\" + _appName;
    }
    appLogFile = appLogDir + "/" + logName;

#else

    if (getuid()==0)
    {
        appLogDir = "/var/log/" + _appName;
    }
    else
    {
        if (isUsingSqliteLog())
        {
            const char *homedir;
            homedir = getpwuid(getuid())->pw_dir;
            appLogDir = string(homedir) + string("/.") +  _appName;
            if (access(appLogDir.c_str(),R_OK)) mkdir(appLogDir.c_str(),0700);
            appLogDir = appLogDir + string("/log");
            if (access(appLogDir.c_str(),R_OK)) mkdir(appLogDir.c_str(),0700);
        }
    }
    appLogFile = appLogDir + "/" + logName;
#endif

#ifndef NOSQLITE
    ppDb = nullptr;
#endif

    initialize();
}

AppLog::~AppLog()
{
    std::unique_lock<std::mutex> lock(mt);
#ifndef NOSQLITE
    if (ppDb)
    {
        sqlite3_close(ppDb);
        ppDb = nullptr;
        logMode = logMode & ~((uint32_t)LOG_MODE_SYSLOG);
    }
#endif
    if (isUsingSyslog())
    {
#ifndef _WIN32
        closelog();
#endif
    }
}

void AppLog::activateModuleOutput(const string &moduleName)
{
    std::unique_lock<std::mutex> lock(mutexModulesOutputExclusionSet);
    modulesOutputExclusion.erase(moduleName);
}

void AppLog::deactivateModuleOutput(const string &moduleName)
{
    std::unique_lock<std::mutex> lock(mutexModulesOutputExclusionSet);
    modulesOutputExclusion.insert(moduleName);
}

void AppLog::printDate(FILE *fp)
{
    char xdate[64]="";
    time_t x = time(nullptr);
    struct tm *tmp = localtime(&x);
#ifndef _WIN32
    strftime(xdate, 64, "%Y-%m-%dT%H:%M:%S%z", tmp);
#else
    strftime(xdate, 64, "%Y-%m-%dT%H:%M:%S", tmp);
#endif
    fprintf(fp,"%s%s", xdate, standardLogSeparator.c_str());
}

std::string getAlignedValue(const std::string & value, size_t sz)
{
    if (value.size()>=sz) return value;
    else
    {
        char * tmpValue = new char[sz+2];
        memset(tmpValue,0,sz+2);
        memset(tmpValue,' ',sz);
        memcpy(tmpValue,value.c_str(),value.size());
        std::string r;
        r=tmpValue;
        delete [] tmpValue;
        return r;
    }
}


void AppLog::printStandardLog(FILE *fp, string module, string user, string ip, const char *buffer, eLogColors color, const char * logLevelText)
{
    if (true)
    {
        std::unique_lock<std::mutex> lock(mutexModulesOutputExclusionSet);
        if (modulesOutputExclusion.find(module)!=modulesOutputExclusion.end()) return;
    }

    if (!usingAttributeName)
    {
        if (module.empty()) module="-";
        if (user.empty()) user="-";
        if (ip.empty()) ip="-";
    }

    std::string logLine;

    if (usingAttributeName)
    {
        if ((module.empty() && printEmptyFields) || !module.empty())
            logLine += "MODULE=" + getAlignedValue(module,moduleAlignSize) + standardLogSeparator;
        if ((ip.empty() && printEmptyFields) || !ip.empty())
            logLine += "IPADDR=" + getAlignedValue(ip,INET_ADDRSTRLEN) + standardLogSeparator;
        if ((user.empty() && printEmptyFields) || !user.empty())
            logLine += "USER=" + getAlignedValue(user,userAlignSize) +  standardLogSeparator;
        if ((!buffer[0] && printEmptyFields) || buffer[0])
            logLine += "LOGDATA=\"" + std::string(buffer) + "\"";
    }
    else
    {
        if ((module.empty() && printEmptyFields) || !module.empty())
            logLine += getAlignedValue(module,moduleAlignSize) + standardLogSeparator;
        if ((ip.empty() && printEmptyFields) || !ip.empty())
            logLine +=  getAlignedValue(ip,INET_ADDRSTRLEN) + standardLogSeparator;
        if ((user.empty() && printEmptyFields) || !user.empty())
            logLine +=  getAlignedValue(user,userAlignSize) +  standardLogSeparator;
        if ((!buffer[0] && printEmptyFields) || buffer[0])
            logLine += std::string(buffer);
    }

    if (usingPrintDate)
    {
        printDate(fp);
        fprintf(fp, "%s", standardLogSeparator.c_str());
    }

    if (usingColors)
    {
        if (usingAttributeName) fprintf(fp, "LEVEL=");
        switch (color)
        {
        case LOG_COLOR_NORMAL:
            fprintf(fp,"%s", getAlignedValue(logLevelText,6).c_str()); break;
        case LOG_COLOR_BOLD:
            printColorBold(fp,getAlignedValue(logLevelText,6).c_str()); break;
        case LOG_COLOR_RED:
            printColorRed(fp,getAlignedValue(logLevelText,6).c_str()); break;
        case LOG_COLOR_GREEN:
            printColorGreen(fp,getAlignedValue(logLevelText,6).c_str()); break;
        case LOG_COLOR_BLUE:
            printColorBlue(fp,getAlignedValue(logLevelText,6).c_str()); break;
        case LOG_COLOR_PURPLE:
            printColorPurple(fp,getAlignedValue(logLevelText,6).c_str()); break;
        }
        fprintf(fp, "%s", standardLogSeparator.c_str());
    }
    else
    {
        fprintf(fp, "%s", getAlignedValue(logLevelText,6).c_str());
        fprintf(fp, "%s", standardLogSeparator.c_str());
    }

    fprintf(fp, "%s\n",  logLine.c_str());
    fflush(fp);
}

void AppLog::log(const string &module, const string &user, const string &ip,eLogLevels logSeverity, const uint32_t & outSize, const char * fmtLog, ...)
{
    std::unique_lock<std::mutex> lock(mt);
    char * buffer = new char [outSize];
    if (!buffer) return;
    buffer[outSize-1] = 0;

    std::list<sLogElement> r;

    // take arguments...
    va_list args;
    va_start(args, fmtLog);
    vsnprintf(buffer, outSize-2, fmtLog, args);

    if (isUsingSyslog())
    {
#ifndef _WIN32
        if (logSeverity == LOG_LEVEL_INFO)
            syslog( LOG_INFO,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_WARN)
            syslog( LOG_WARNING,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            syslog( LOG_CRIT, "%s",buffer);
        else if (logSeverity == LOG_LEVEL_ERR)
            syslog( LOG_ERR, "%s",buffer);
#endif
    }

    if (isUsingWindowsEventLog())
    {
        //TODO:
    }

    if (isUsingStandardLog())
    {
        if (logSeverity == LOG_LEVEL_INFO)
            printStandardLog(stdout,module,user,ip,buffer,LOG_COLOR_BOLD,"INFO");
        else if (logSeverity == LOG_LEVEL_WARN)
            printStandardLog(stdout,module,user,ip,buffer,LOG_COLOR_BLUE,"WARN");
        else if ((logSeverity == LOG_LEVEL_DEBUG || logSeverity == LOG_LEVEL_DEBUG1) && debug)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_GREEN,"DEBUG");
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_RED,"CRIT");
        else if (logSeverity == LOG_LEVEL_ERR)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_PURPLE,"ERR");
    }
#ifndef NOSQLITE
    if (isUsingSqliteLog() && ppDb)
    {
        unsigned int log_severity = (unsigned int) logSeverity;
        std::string severity = to_string(log_severity);

        sqliteExecQueryVA("INSERT INTO logs_v1 (date,severity,module,user,ip,message) VALUES(DateTime('now'),?,?,?,?,?);", 5, severity.c_str(), module.c_str(), user.c_str(), ip.c_str(), buffer);
    }
#endif
    va_end(args);
    delete [] buffer;
}

void AppLog::log2(const string &module, const string &user, const string &ip, eLogLevels logSeverity, const char *fmtLog, ...)
{
    std::unique_lock<std::mutex> lock(mt);
    char buffer[8192];
    buffer[8191] = 0;

    std::list<sLogElement> r;

    // take arguments...
    va_list args;
    va_start(args, fmtLog);
    vsnprintf(buffer, 8192-2, fmtLog, args);

    if (isUsingSyslog())
    {
#ifndef _WIN32
        if (logSeverity == LOG_LEVEL_INFO)
            syslog( LOG_INFO,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_WARN)
            syslog( LOG_WARNING,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            syslog( LOG_CRIT, "%s",buffer);
        else if (logSeverity == LOG_LEVEL_ERR)
            syslog( LOG_ERR, "%s",buffer);
#endif
    }

    if (isUsingWindowsEventLog())
    {
        //TODO:
    }

    if (isUsingStandardLog())
    {
        if (logSeverity == LOG_LEVEL_INFO)
            printStandardLog(stdout,module,user,ip,buffer,LOG_COLOR_BOLD,"INFO");
        else if (logSeverity == LOG_LEVEL_WARN)
            printStandardLog(stdout,module,user,ip,buffer,LOG_COLOR_BLUE,"WARN");
        else if ((logSeverity == LOG_LEVEL_DEBUG || logSeverity == LOG_LEVEL_DEBUG1) && debug)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_GREEN,"DEBUG");
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_RED,"CRIT");
        else if (logSeverity == LOG_LEVEL_ERR)
            printStandardLog(stderr,module,user,ip,buffer,LOG_COLOR_PURPLE,"ERR");
    }
#ifndef NOSQLITE
    if (isUsingSqliteLog() && ppDb)
    {
        unsigned int log_severity = (unsigned int) logSeverity;
        std::string severity = to_string(log_severity);

        sqliteExecQueryVA("INSERT INTO logs_v1 (date,severity,module,user,ip,message) VALUES(DateTime('now'),?,?,?,?,?);", 5, severity.c_str(), module.c_str(), user.c_str(), ip.c_str(), buffer);
    }
#endif
    va_end(args);
}

void AppLog::log1(const string &module, const string &ip, eLogLevels logSeverity, const char *fmtLog, ...)
{
    std::unique_lock<std::mutex> lock(mt);
    char buffer[8192];
    buffer[8191] = 0;

    std::list<sLogElement> r;

    // take arguments...
    va_list args;
    va_start(args, fmtLog);
    vsnprintf(buffer, 8192-2, fmtLog, args);

    if (isUsingSyslog())
    {
#ifndef _WIN32
        if (logSeverity == LOG_LEVEL_INFO)
            syslog( LOG_INFO,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_WARN)
            syslog( LOG_WARNING,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            syslog( LOG_CRIT, "%s",buffer);
        else if (logSeverity == LOG_LEVEL_ERR)
            syslog( LOG_ERR, "%s",buffer);
#endif
    }

    if (isUsingWindowsEventLog())
    {
        //TODO:
    }

    if (isUsingStandardLog())
    {
        if (logSeverity == LOG_LEVEL_INFO)
            printStandardLog(stdout,module,"",ip,buffer,LOG_COLOR_BOLD,"INFO");
        else if (logSeverity == LOG_LEVEL_WARN)
            printStandardLog(stdout,module,"",ip,buffer,LOG_COLOR_BLUE,"WARN");
        else if ((logSeverity == LOG_LEVEL_DEBUG || logSeverity == LOG_LEVEL_DEBUG1) && debug)
            printStandardLog(stderr,module,"",ip,buffer,LOG_COLOR_GREEN,"DEBUG");
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            printStandardLog(stderr,module,"",ip,buffer,LOG_COLOR_RED,"CRIT");
        else if (logSeverity == LOG_LEVEL_ERR)
            printStandardLog(stderr,module,"",ip,buffer,LOG_COLOR_PURPLE,"ERR");
    }
#ifndef NOSQLITE
    if (isUsingSqliteLog() && ppDb)
    {
        unsigned int log_severity = (unsigned int) logSeverity;
        std::string severity = to_string(log_severity);

        sqliteExecQueryVA("INSERT INTO logs_v1 (date,severity,module,user,ip,message) VALUES(DateTime('now'),?,?,?,?,?);", 5, severity.c_str(), module.c_str(), "", ip.c_str(), buffer);
    }
#endif
    va_end(args);
}

void AppLog::log0(const string &module, eLogLevels logSeverity, const char *fmtLog, ...)
{
    std::unique_lock<std::mutex> lock(mt);
    char buffer[8192];
    buffer[8191] = 0;

    std::list<sLogElement> r;

    // take arguments...
    va_list args;
    va_start(args, fmtLog);
    vsnprintf(buffer, 8192-2, fmtLog, args);

    if (isUsingSyslog())
    {
#ifndef _WIN32
        if (logSeverity == LOG_LEVEL_INFO)
            syslog( LOG_INFO,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_WARN)
            syslog( LOG_WARNING,"%s", buffer);
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            syslog( LOG_CRIT, "%s",buffer);
        else if (logSeverity == LOG_LEVEL_ERR)
            syslog( LOG_ERR, "%s",buffer);
#endif
    }

    if (isUsingWindowsEventLog())
    {
        //TODO:
    }

    if (isUsingStandardLog())
    {
        if (logSeverity == LOG_LEVEL_INFO)
            printStandardLog(stdout,module,"","",buffer,LOG_COLOR_BOLD,"INFO");
        else if (logSeverity == LOG_LEVEL_WARN)
            printStandardLog(stdout,module,"","",buffer,LOG_COLOR_BLUE,"WARN");
        else if ((logSeverity == LOG_LEVEL_DEBUG || logSeverity == LOG_LEVEL_DEBUG1) && debug)
            printStandardLog(stderr,module,"","",buffer,LOG_COLOR_GREEN,"DEBUG");
        else if (logSeverity == LOG_LEVEL_CRITICAL)
            printStandardLog(stderr,module,"","",buffer,LOG_COLOR_RED,"CRIT");
        else if (logSeverity == LOG_LEVEL_ERR)
            printStandardLog(stderr,module,"","",buffer,LOG_COLOR_PURPLE,"ERR");
    }
#ifndef NOSQLITE
    if (isUsingSqliteLog() && ppDb)
    {
        unsigned int log_severity = (unsigned int) logSeverity;
        std::string severity = to_string(log_severity);

        sqliteExecQueryVA("INSERT INTO logs_v1 (date,severity,module,user,ip,message) VALUES(DateTime('now'),?,?,?,?,?);", 5, severity.c_str(), module.c_str(), "", "", buffer);
    }
#endif
    va_end(args);
}

bool AppLog::sqliteTableExist(const std::string &table)
{
#ifndef NOSQLITE
    bool ret;
    string xsql = "select sql from sqlite_master where tbl_name=?;";
    sqlite3_stmt * stmt;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, table.c_str(), table.size(), nullptr);
    int s = sqlite3_step(stmt);
    ret = (s == SQLITE_ROW ? true : false);
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
    return ret;
#else
    return false;
#endif
}

bool AppLog::isUsingSyslog()
{
    return (logMode & LOG_MODE_SYSLOG) == LOG_MODE_SYSLOG;
}

bool AppLog::isUsingStandardLog()
{
    return (logMode & LOG_MODE_STANDARD) == LOG_MODE_STANDARD;
}

bool AppLog::isUsingSqliteLog()
{
#ifndef NOSQLITE
    return (logMode & LOG_MODE_SQLITE) == LOG_MODE_SQLITE;
#else
    return false;
#endif
}


void AppLog::printColorBold(FILE *fp, const char *str)
{
#ifdef _WIN32
    PrintColorWin32(fp,FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN,str);
#else
    fprintf(fp,"\033[1m%s\033[0m", str);
#endif
}

void AppLog::printColorBlue(FILE *fp, const char *str)
{
#ifdef _WIN32
    PrintColorWin32(fp,FOREGROUND_INTENSITY|FOREGROUND_BLUE,str);
#else
    fprintf(fp,"\033[1;34m%s\033[0m", str);
#endif
}

void AppLog::printColorGreen(FILE *fp, const char *str)
{
#ifdef _WIN32
    PrintColorWin32(fp,FOREGROUND_INTENSITY|FOREGROUND_GREEN,str);
#else
    fprintf(fp,"\033[1;32m%s\033[0m", str);
#endif
}

void AppLog::printColorRed(FILE *fp, const char *str)
{
#ifdef _WIN32
    PrintColorWin32(fp,FOREGROUND_INTENSITY|FOREGROUND_RED,str);
#else
    fprintf(fp,"\033[1;31m%s\033[0m", str);
#endif
}

void AppLog::printColorPurple(FILE *fp, const char *str)
{
#ifdef _WIN32
    PrintColorWin32(fp,FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE,str);
#else
    fprintf(fp,"\033[1;35m%s\033[0m", str);
#endif
}

void AppLog::printColorForWin32(FILE *fp, unsigned short color, const char *str)
{
#ifdef _WIN32
    DWORD ouputHandleSrc = fp==stdout?STD_OUTPUT_HANDLE:STD_ERROR_HANDLE;
    HANDLE outputHandle = GetStdHandle(ouputHandleSrc);
    CONSOLE_SCREEN_BUFFER_INFO *ConsoleInfo = new CONSOLE_SCREEN_BUFFER_INFO();
    GetConsoleScreenBufferInfo(outputHandle, ConsoleInfo);
    WORD OriginalColors = ConsoleInfo->wAttributes;
    delete ConsoleInfo;
    SetConsoleTextAttribute(outputHandle, color);
    fprintf(fp, "%s", str );
    SetConsoleTextAttribute(outputHandle, OriginalColors);
#endif
}

bool AppLog::sqliteExecQuery(const std::string& query)
{
#ifndef NOSQLITE
    const char *tail;
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare_v2(ppDb, query.c_str(), query.length(), &stmt, &tail);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        fprintf(stderr, "Log SQL error: %s\n", sqlite3_errmsg(ppDb));
        return false;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return true;
#else
    return false;
#endif
}

bool AppLog::sqliteExecQueryVA(const std::string& query, int _va_size, ...)
{
#ifndef NOSQLITE
    const char *tail;
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare_v2(ppDb, query.c_str(), query.length(), &stmt, &tail);

    va_list args;
    va_start(args, _va_size);

    for (int i = 0; i < _va_size; i++)
    {
        const char *val_to_bind = va_arg(args, const char *);
        sqlite3_bind_text(stmt, i + 1, val_to_bind, strlen(val_to_bind) + 1, SQLITE_TRANSIENT);
    }

    va_end(args);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
        return false;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return true;
#else
    return false;
#endif
}

bool AppLog::isUsingWindowsEventLog()
{
    return (logMode & LOG_MODE_WINEVENTS) == LOG_MODE_WINEVENTS;
}

void AppLog::dropLog()
{
#ifndef NOSQLITE
    sqliteExecQuery("DELETE FROM logs_v1 WHERE 1=1;");
#endif
}

void AppLog::initialize()
{
    if (isUsingSyslog())
    {
#ifndef _WIN32
        openlog( nullptr, LOG_PID, LOG_LOCAL5);
#else
        fprintf(stderr,"SysLog Not implemented on WIN32, don't use.");
#endif
    }
    if (isUsingStandardLog())
    {
        // do nothing...
    }
    if (isUsingWindowsEventLog())
    {
        //TODO: future work.
    }
#ifndef NOSQLITE
    if (isUsingSqliteLog())
    {
        if (access(appLogDir.c_str(), R_OK))
        {
#ifdef _WIN32
            if (mkdir(appLogDir.c_str()) == -1)
#else
            if (mkdir(appLogDir.c_str(), 0700) == -1)
#endif
            {
                fprintf(stderr, " [+] ERROR (@std_log)> Unable to create log dir (%s)\n", appLogDir.c_str());
            }
        }
        if (!access(appLogDir.c_str(), W_OK))
        {
            int rc;
            rc = sqlite3_open(appLogFile.c_str(), &ppDb);
            if (rc)
            {
                fprintf(stderr, " [+] ERROR (@std_log)> Unable to create/open log file (%s) - %s\n", appLogFile.c_str(), sqlite3_errmsg(ppDb));
            }
            else
            {
                if (!sqliteTableExist("logs_v1"))
                {
                    sqliteExecQuery("CREATE TABLE \"logs_v1\" (\"id\" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , \"date\" DATETIME NOT NULL , \"severity\" INTEGER NOT NULL , \"module\" VARCHAR(256) NOT NULL, \"user\" VARCHAR(256), \"ip\" VARCHAR(46), \"message\" TEXT NOT NULL );");
                    sqliteExecQuery("CREATE UNIQUE INDEX \"idx_logs_v1_id\" ON \"logs_v1\" (\"id\" ASC);");
                    sqliteExecQuery("CREATE INDEX \"idx_logs_v1_date\" ON \"logs_v1\" (\"date\" ASC);");
                    sqliteExecQuery("CREATE INDEX \"idx_logs_v1_module\" ON \"logs_v1\" (\"module\" ASC);");
                    sqliteExecQuery("CREATE INDEX \"idx_logs_v1_user\" ON \"logs_v1\" (\"user\" ASC);");
                    sqliteExecQuery("CREATE INDEX \"idx_logs_v1_ip\" ON \"logs_v1\" (\"ip\" ASC);");
                    sqliteExecQuery("CREATE INDEX \"idx_logs_v1_severity\" ON \"logs_v1\" (\"severity\" ASC);");

                    // now we have the tables ;)
                }
            }
        }
    }
#endif
}

uint32_t AppLog::getModuleAlignSize() const
{
    return moduleAlignSize;
}

void AppLog::setModuleAlignSize(const uint32_t &value)
{
    moduleAlignSize = value;
}

uint32_t AppLog::getUserAlignSize() const
{
    return userAlignSize;
}

void AppLog::setUserAlignSize(const uint32_t &value)
{
    userAlignSize = value;
}

bool AppLog::getPrintEmptyFields() const
{
    return printEmptyFields;
}

void AppLog::setPrintEmptyFields(bool value)
{
    printEmptyFields = value;
}

bool AppLog::getUsingColors() const
{
    return usingColors;
}

void AppLog::setUsingColors(bool value)
{
    usingColors = value;
}

bool AppLog::getUsingAttributeName() const
{
    return usingAttributeName;
}

void AppLog::setUsingAttributeName(bool value)
{
    usingAttributeName = value;
}

std::string AppLog::getStandardLogSeparator() const
{
    return standardLogSeparator;
}

void AppLog::setStandardLogSeparator(const std::string &value)
{
    standardLogSeparator = value;
}

bool AppLog::getUsingPrintDate() const
{
    return usingPrintDate;
}

void AppLog::setUsingPrintDate(bool value)
{
    usingPrintDate = value;
}

void AppLog::setDebug(bool value)
{
    std::unique_lock<std::mutex> lock(mt);
    debug = value;
}

unsigned int AppLog::getLogLastID()
{
    std::unique_lock<std::mutex> lock(mt);
#ifndef NOSQLITE
    unsigned int x = 0;
    string xsql = "SELECT id FROM logs_v1 ORDER BY id DESC LIMIT 1";

    sqlite3_stmt * stmt;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);

    int s = sqlite3_step(stmt);
    if (s == SQLITE_ROW)
    {
        x = sqlite3_column_int(stmt, 0);
    }
    else
    {
        fprintf(stderr, "Get IDS failed (using 0).\n");
        exit(1);
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return x;
#else
    return 0;
#endif
}

std::list<sLogElement> AppLog::getLogView(unsigned int id_from, unsigned int id_to, const std::string& filter, eLogLevels logLevelFilter)
{
    std::unique_lock<std::mutex> lock(mt);

    std::list<sLogElement> r;
#ifndef NOSQLITE
    string xsql;

    if (logLevelFilter == LOG_LEVEL_ALL)
        xsql = "SELECT id,date as TEXT,severity,module,user,ip,message AS TEXT FROM logs_v1 WHERE id >= '" + to_string(id_from) + "' and id <= '" + to_string(id_to) + "';";
    else
        xsql = "SELECT id,date as TEXT,severity,module,user,ip,message AS TEXT FROM logs_v1 WHERE id >= '" + to_string(id_from) + "' AND id <= '" + to_string(id_to) + "' and severity = '" + to_string((unsigned int) logLevelFilter) + "';";

    sqlite3_stmt * stmt;
    sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, nullptr);

    for (;;)
    {
        int s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            sLogElement rx;
            rx.id = sqlite3_column_int(stmt, 0);
            rx.date = (const char *)sqlite3_column_text(stmt, 1);
            rx.severity = sqlite3_column_int(stmt, 2);
            rx.module = (const char *)sqlite3_column_text(stmt, 3);
            rx.user = (const char *)sqlite3_column_text(stmt, 4);
            rx.ip = (const char *)sqlite3_column_text(stmt, 5);
            rx.message = (const char *)sqlite3_column_text(stmt, 6);

            r.push_back(rx);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
        else
        {
            fprintf(stderr, "Get OIDS failed.\n");
            exit(1);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);
#endif
    return r;
}
