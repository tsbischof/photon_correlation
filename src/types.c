#include "types.h"

char* ctime32(time32 *mytime) {
#ifdef __i386__
	return(ctime(mytime));
#endif

#ifdef __x86_64__
	time_t mytime64;
	mytime64 = (int64)*mytime;
	return(ctime(&mytime64));
#endif
}
