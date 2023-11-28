#include "errors.h"

void terminate_w_error(const char* error, const char* details){
	fputs(error, stderr);
	fputs(": ", stderr);
	fputs(details, stderr);
	fputs("\n", stderr);
	exit(1);
}
