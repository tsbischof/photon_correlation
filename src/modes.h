#ifndef MODES_H_
#define MODES_H_

#define MODE_UNKNOWN               -1
#define MODE_INTERACTIVE            0
#define MODE_T2                     2
#define MODE_T3                     3
#define MODE_VECTOR             0xBEEF

#define MODE_AS_T2              0xDEAD

#define DEFAULT_BIN_WIDTH(x) x == MODE_T2 ? 50000000000 : 100000

int mode_parse(int *mode, char const *mode_string);

#endif
