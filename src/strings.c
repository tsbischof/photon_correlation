#include <string.h>
#include <stdlib.h>

/* In principle, this is handled in POSIX, but gcc does not seem to like it.
 * Defined here for convenience.
 */

char *strdup(const char *src) {
	char *dst = (char *)malloc(strlen(src)+1);
	if ( dst == NULL ) {
		return(NULL);
	}
	strcpy(dst, src);
	return(dst);
}
