#ifndef directives_h
#define directives_h

#include "module.h"

void Module_register_default_directives(Module *state);

void *Library_search(Module *state, Token *module, Token *name);

#endif
