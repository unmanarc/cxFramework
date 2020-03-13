#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdint.h>
#include <limits>

#define MAX_SIZE_T std::numeric_limits<std::size_t>::max()

#define CHECK_UINT_OVERFLOW_SUM(a,b) (a+b<a || a+b<b)
#define CHECK_UINT_OVERFLOW_REM(a,b) (b>a)

#define KB_MULT (1024)
#define MB_MULT (KB_MULT*1024)
#define GB_MULT (MB_MULT*1024)
#define TB_MULT (GB_MULT*1024)

bool icharcmp(unsigned char c1,unsigned  char c2);
int memicmp2(const void *s1, const void *s2, const uint64_t &n, const bool &caseSensitive);

int memcmp64(const void *s1, const void *s2, uint64_t n);
void *memcpy64(void *dest, const void *src, uint64_t n);
void *memmove64(void *dest, const void *src, uint64_t n);


#endif // COMMON_H

