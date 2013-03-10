#include "run.h"
#include "options.h"
#include "permutations.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"Constructs and prints combinations, for testing purposes.",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_OUT, 
			OPT_ORDER, OPT_EOF}};

	return(run(&program_options, combinations_dispatch, argc, argv));
}
