#ifndef TYPES_H_
#define TYPES_H_

#include <inttypes.h>
#include <stdint.h>
#include <time.h>

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
#ifdef __i386__
typedef time_t time32;
#endif
#ifdef __x86_64__
typedef int32_t time32;
#endif

typedef float float32;
typedef double float64;

char* ctime32(time32 *mytime);

#endif
