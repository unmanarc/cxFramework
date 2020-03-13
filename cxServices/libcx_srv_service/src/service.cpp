#include "service.h"

// STD:
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
// STL:
#include <iostream>
#include <fstream>
// SYS:
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

using namespace std;

// LOCK:
static int lockfd = -1;

static void daemonize();
void pidCheck();
void exitRoutine(int32_t d, void *);

int main(int argc, char *argv[])
{
    // Get program name from program path.
    globalArgs.initProgramName(argv[0]);

    // Local default cmd options...
    globalArgs.addCommandLineOption("Service Options", 'd', "daemon" , "Run as daemon."         , "0", ABSTRACT_BOOL );
    globalArgs.addCommandLineOption("Other Options"  , 'v', "verbose", "Set verbosity level."   , "0", ABSTRACT_UINT8 );
    globalArgs.addCommandLineOption("Other Options"  , 'h', "help"   , "Show information usage.", "0", ABSTRACT_BOOL  );

    // Init vars...
    _initvars(argc,argv, &globalArgs);
    // Print program description:
    globalArgs.printProgramHeader();
    // Parse program options.
    if (!globalArgs.parseCommandLineOptions(argc,argv))
    {
        cout << "ERR: Failed to Load CMD Line Parameters." << endl << flush;
        return -2;
    }

    // Check if it's a help program:
    if (globalArgs.getCommandLineOptionBooleanValue(globalArgs.getDefaultHelpOption()))
    {
        globalArgs.printHelp();
        return 0;
    }

    // Load/Prepare the configuration based in command line arguments.
    if (!_config(argc,argv,&globalArgs))
    {
        cout << "ERR: Failed to Load Configuration." << endl << flush;
        return -1;
    }

    int r = 0;
    if (!globalArgs.getCommandLineOptionBooleanValue(globalArgs.getDefaultDaemonOption()))
    {
        r = _start(argc,argv,&globalArgs);
        if (!globalArgs.isInifiniteWaitAtEnd())
            return r;
        else
        {
            cout << "> This program is running with background threads, press CTRL-C to exit..." << endl << flush;
            for (;;) { sleep(3600); }
        }
    }
    else
    {
        // Initialize the logging interface
        openlog( globalArgs.getDaemonName().c_str(), LOG_PID, LOG_LOCAL5);
        syslog( LOG_INFO, "Initiating as service...");

        // Daemonize:
        daemonize();

        // Locked daemon -- check pid
        pidCheck();

        // Allow this application to be killed and setup an exit routine.
        signal(SIGHUP, SIG_DFL);
        on_exit(exitRoutine, nullptr);

        r = _start(argc,argv,&globalArgs);

        if (!globalArgs.isInifiniteWaitAtEnd())
        {
            // Finish up.
            syslog( LOG_NOTICE, "terminated (%d) by program execution", r);
            closelog();
            return r;
        }
        else
        {
            syslog( LOG_NOTICE, "This program (%d) is running with background threads, send kill signal to terminate it.", getpid());
            for (;;) { sleep(3600); }
        }
    }
}

static void child_handler(int signum)
{
    switch (signum)
    {
    case SIGALRM:
        cerr << globalArgs.getDaemonName() << " child handler: SIGALRM" << endl << flush;
        _exit(EXIT_FAILURE);
    case SIGUSR1:
        _exit(EXIT_SUCCESS);
    case SIGCHLD:
        cerr << globalArgs.getDaemonName() << " child handler: SIGCHLD" << endl << flush;
        _exit(EXIT_FAILURE);
    }
}

static int get_lock()
{
    struct flock lplock;
    if (access("/var/lock/subsys", R_OK))
    {
        mkdir("/var/lock/subsys", 0755);
    }

    string lockFile = "/var/lock/subsys/" + globalArgs.getDaemonName();

    if ((lockfd = open(lockFile.c_str(), O_CREAT | O_RDWR, 0700)) < 0)
    {
        return (0);
    }

    memset(&lplock, 0, sizeof(lplock));
    lplock.l_type = F_WRLCK;
    lplock.l_pid = getpid();

    // Lock this file to my PID.
    if (fcntl(lockfd, F_SETLK, &lplock) < 0)
    {
        return (0);
    }
    return (1);
}

static void free_lock(void)
{
    if (lockfd >= 0)
        (void) close(lockfd);
}

static void daemonize()
{
    pid_t pid, sid, parent;

    // already a daemon
    if (getppid() == 1)
        return;

    // Trap signals that we expect to recieve
    signal(SIGCHLD, child_handler);
    signal(SIGUSR1, child_handler);
    signal(SIGALRM, child_handler);

    // Fork off the parent process
    pid = fork();
    if (pid < 0)
    {
        syslog( LOG_ERR, "unable to fork daemon, code=%d [%s]", errno, strerror(errno));
        _exit(EXIT_FAILURE);
    }

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        // Wait for confirmation from the child via SIGTERM or SIGCHLD, or for two seconds to elapse (SIGALRM).  pause() should not return.
        alarm(2);
        pause();
        _exit(EXIT_FAILURE);
    }

    // Executing as child process
    parent = getppid();

    // disable some signals:
    signal(SIGCHLD, SIG_DFL); // Child process dies.
    signal(SIGTSTP, SIG_IGN); // TTY signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);  // Ignore soft termination
    signal(SIGTERM, SIG_DFL); // die on hard terminate signal

    umask(0); // Change the mask

    // Create a new session ID for the child process
    sid = setsid();
    if (sid < 0)
    {
        syslog( LOG_ERR, "unable to create a new session, code %d (%s)", errno, strerror(errno));
        _exit(EXIT_FAILURE);
    }

    // Tell the parent process that we are A-okay
    kill(parent, SIGUSR1);

    // Create the lock file as the current proccess, and if it does not work get out of here.
    if (get_lock() == 0)
    {
        if (getuid() == 0)
            cerr << "ERR: " << globalArgs.getDaemonName() << " already running..." << endl << flush;
        else
            cerr << "ERR: " << globalArgs.getDaemonName() << " insufficient privileges (uid=0 required, running as: " <<  getuid() << ")..." << endl << flush;
        fflush(stdout);
        syslog( LOG_ERR, "unable to create lock file.");
        _exit(EXIT_FAILURE);
    }

    // Check root
    if (getuid() != 0)
    {
        printf("root(0) required.\n");
        _exit(EXIT_FAILURE);
    }

    // Redirect standard files to /dev/null and log files.

    string logFile_out = "/var/log/" + globalArgs.getDaemonName() + "/out.log";
    string logFile_err = "/var/log/" + globalArgs.getDaemonName() + "/err.log";

    freopen("/dev/null", "r", stdin);
    freopen( logFile_out.c_str(), "w", stdout);
    freopen( logFile_err.c_str(), "w", stderr);
}

void pidCheck()
{
    string pidFile = "/var/run/" + globalArgs.getDaemonName() + ".pid";

    if (!access(pidFile.c_str(), F_OK))
    {
        remove(pidFile.c_str());
    }
    ofstream runFile;
    runFile.open(pidFile.c_str());
    runFile << to_string(getpid());
    runFile.close();
}

void exitRoutine(int32_t d, void *)
{
    string pidFile = "/var/run/" + globalArgs.getDaemonName() + ".pid";

    fprintf(stderr, "Finalizing (%s) (%d) - pid %d.\n", globalArgs.getDaemonName().c_str(), d, getpid());
    fflush(stdout);
    remove(pidFile.c_str());
    free_lock();
}

