#include "module.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "directives.h"
#include "grammar.h"
#include <string.h>
#include "error.h"

Module *Module_create(const char *name, size_t max_code_size)
{
    Module *state = calloc(1, sizeof(Module));
    assert(state && "Memory alloc failure for Module.");
    state->curline = 1;
    state->module_name = strdup(name);
    state->max_code_size = max_code_size;

    return state;
}

Module *Module_resolve_module(Module *state, Token *ident)
{
    if(ident->len == 4 && strncmp(ident->start, "self", 4) == 0) {
        return state;
    } else {
        return (Module *)Naming_search(state->imports, ident->start, ident->len);
    }
}

Token *Module_resolve_function_data(Function *func, Token *ident)
{
    Token *expr = Naming_search(func->data, ident->start, ident->len);
    if(expr) {
        *ident = *expr;  // copy it over reusing the ident
        return ident;
    } else {
        return NULL;
    }
}

Token *Module_outside_function_data(Module *state, Token *func_ref, Token *ident)
{
    Token *expr = NULL;
    Function *func = Function_find(state, func_ref->start, func_ref->len);

    if(func) expr = Module_resolve_function_data(func, ident);

    if(!expr) {
        die(state, "function %.*s doesn't have data named %.*s",
                func_ref->len, func_ref->start, ident->len, ident->start);
    }

    free(func_ref);
    return expr;
}

Token *Module_resolve_data(Module *state, Token *ident)
{
    Token *expr = Module_resolve_function_data(state->current, ident);

    if(expr) {
        return expr;
    } else {
        expr = Function_resolve_param(state, state->current, ident); 

        if(expr) {
            return expr;
        } else {
            // try to find it in the module
            expr = Naming_search(state->data, ident->start, ident->len);

            if(expr) {
                *ident = *expr;
                return ident;
            } else {
                die(state, "failed to resolve %.*s, not a parameter or constant\n", 
                        ident->len, ident->start);
                return NULL;
            }
        }
    }
}

Token *Module_resolve_function(Module *state, Token *module, Token *ident)
{
    void *result = NULL;

    Module *mod = Module_resolve_module(state, module);
    if(mod) {
        Function *func = Function_find(mod, ident->start, ident->len);
        if(func) {
            result = func->code;
        }
    } else {
        result = Library_search(state, module, ident);
    }

    if(result != NULL) {
        // it's a function defined in this module
        ident->value = (unsigned long)result;
    } else {
        // it's not found so not a possible ident
        die(state, "did not find function %.*s in a library or module named %.*s.", 
                ident->len, ident->start,
                module->len, module->start);
    }

    free(module);
    return ident;
}

void Module_call_directive(Module *state, Token *ident, Parameters *params)
{
   Directive *to_call = (Directive *)Naming_search(state->directives, ident->start, ident->len);

   if(to_call) {
       to_call->call(state, params);
   } else {
       die(state, "referenced directive %.*s which doesn't exist.", ident->len, ident->start);
   }

   Parameters_destroy(params);
   free(ident);
}

int Module_register_directive(Module *state, const char *name, int length, 
        Module_directive_cb call, Module_directive_cb destroy)
{
    Directive *d = calloc(1, sizeof(Directive));
    assert(d && "Memory allocation failed for Directive.");
    
    if(Naming_search(state->directives, name, length)) {
        die(state, "Directive %.*s already exists in the directive list.", length, name);
        return 0;
    } else {
        d->name = name;
        d->len = length;
        d->call = call;
        d->destroy = destroy;

        state->directives = Naming_insert(state->directives, name, length, d);

        return 1;
    }
}

void Module_create_constant(Module *state, Token *ident, Token *expr)
{
    if(Naming_search(state->data, ident->start, ident->len)) {
        die(state, "you already defined a constant named %.*s in this module.", 
                ident->len, ident->start);
    } else {
        state->data = Naming_insert(state->data, ident->start, ident->len, expr);
        assert(state->data && "Unable to insert an element into the module data store.");

        if(expr->id == TK_STR) {
            // need to disconnect this from its actual source for reference later.
            expr->value = (unsigned long)strndup(expr->start, expr->len);
        }
    }
}

void Module_create_function_constant(Module *state, Token *ident, Token *expr)
{
    if(Naming_search(state->current->data, ident->start, ident->len)) {
        die(state, "you already defined a function constant named %.*s", 
                ident->len, ident->start);
    } else {
        state->current->data = Naming_insert(state->current->data, ident->start, ident->len, expr);
        assert(state->current->data && "Unable to insert an element into the module data store.");

        if(expr->id == TK_STR) {
            // need to disconnect this from its actual source for reference later.
            expr->value = (unsigned long)strndup(expr->start, expr->len);
        }
    }
}

void Module_dump_directives(Module *state, FILE *out)
{
    void dump(void *p, void *ignored) {
        Directive *d = p;
        fprintf(out, "directive: %.*s\n", d->len, d->name);
    }

    Naming_traverse(state->directives, dump, NULL);
}

void Module_destroy_directives(Module *state)
{
    static Module *state_holder = NULL;

    state_holder = state;  // OSX needs it like this, must refactor

    void lib_destroy(void *p) {
        Directive *d = (Directive *)p;
        if(d->destroy) d->destroy(state_holder, NULL);
        free(d);
    }

    Naming_destroy(state->directives, lib_destroy);
}


void Module_destroy(Module *state)
{
    void data_destroy(Token *expr) {
        if(expr->id == TK_STR) {
            free((char *)expr->value);
        }
        free(expr);
    }

    Naming_destroy(state->imports, (Naming_destroy_cb)Module_destroy);
    Naming_destroy(state->functions, (Naming_destroy_cb)Function_destroy);
    Naming_destroy(state->data, (Naming_destroy_cb)data_destroy);
    Module_destroy_directives(state);
    free(state->module_name);
    free(state);
}


