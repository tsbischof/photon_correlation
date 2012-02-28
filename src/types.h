#ifndef TYPES_H_
#define TYPES_H_

#include <inttypes.h>
#include <stdint.h>

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef int32_t time32;

typedef float float32;
typedef double float64;

char* ctime32(time32 *mytime);

#endif
