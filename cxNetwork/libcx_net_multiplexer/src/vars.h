#ifndef VARS_H
#define VARS_H

#include <stdint.h>

typedef uint32_t LineID;

#define SOCKET_MULTIPLEXER_VERSION 3

// Sendbuf should be under 65535.
#define MUX_LINE_SENDBUF 8192 // 8Kb read.
#define MUX_LINE_HEAPSIZE (512*1024) // 512Kb.

#define PLUGIN_MAX_DATA 512*1024;

#define JSON_MAX_DATA 8*1024*1024

#endif // VARS_H
