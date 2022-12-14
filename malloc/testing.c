#include "testing.h"
#include <stdio.h>
#include "printfmt.h"

static int _failure_count;
static int _exit_count;

void real_print_test(const char* mensaje, bool ok,
		     const char* file, int line, const char* failed_expr) {
    if (ok) {
	printfmt("%s... OK\n", mensaje);
    _exit_count += ok;
    } else {
	printfmt("%s: ERROR\n"  "%s:%d: %s\n",
	       mensaje, file, line, failed_expr);
    }
    fflush(stdout);
    _failure_count += !ok;

}

int failure_count() {
    return _failure_count;
}

int exit_count() {
    return _exit_count;
}
