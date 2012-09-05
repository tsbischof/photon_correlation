#ifndef STRTOLL_H_
#define STRTOLL_H_

long long
strtoll_l(const char * __restrict nptr, char ** __restrict endptr, int base);
long long
strtoll(const char * __restrict nptr, char ** __restrict endptr, int base);

#endif
