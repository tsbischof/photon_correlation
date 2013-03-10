#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <inttypes.h>

typedef int (*compare_t)(const void *, const void *);

#define true 1
#define false 0

#define MAX(x, y) (x < y ? y : x);
#define ABS(x) ( x >= 0 ? x : -x );

#endif
