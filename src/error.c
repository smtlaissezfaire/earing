#include "error.h"
#include <stdarg.h>

static void print_location(Module *state)
{
    if(state) {
        printf("%s:%d: ", state->module_name, state->curline);
        if(state->current) {
            printf("in function %.*s, ", state->current->len, state->current->name);
        }
    }
}

void error(Module *state, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    print_location(state);
    vprintf(format, ap);
    printf("\n");
}


void die(Module *state, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    print_location(state);
    vprintf(format, ap);
    printf("\n");

    if(state) Module_destroy(state);
    exit(1);
}
