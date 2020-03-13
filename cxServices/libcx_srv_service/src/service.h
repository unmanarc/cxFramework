#ifndef SERVICE_H
#define SERVICE_H

#include "globalarguments.h"
#include <cx_mem_abstracts/abstractvar.h>
#include <unistd.h>

static GlobalArguments globalArgs;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void _initvars(int argc, char *argv[], GlobalArguments * globalArguments);
bool _config(int argc, char *argv[], GlobalArguments * globalArguments);
int _start(int argc, char *argv[], GlobalArguments * globalArguments);

#endif // SERVICE_H
