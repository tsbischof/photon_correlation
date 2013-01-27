#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <inttypes.h>

#define strtou64 strtoull
#define strtoi64 strtoll
#define strtou32 strtoul

#define PRIf64 "lf"
#define PRIf32 "f"
typedef double float64_t;
typedef float float32_t;

int64_t i64abs(int64_t i);

#endif
