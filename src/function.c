#include "module.h"
#include "sglib.h"
#include "directives.h"
#include <stdlib.h>
#include "grammar.h"
#include "error.h"

#define FUNCTION_COMPARE(x,y) (strncmp(x->name, y->name, (x->len < y->len ? x->len : y->len) - (y->len - x->len)))

const char *OpType_strings[] = {
   "int", "uint", "long", "ulong",
   "ptr", "float", "double", "void",
   "char", "uchar", "short", "ushort"
};

const char *OpType_to_str(OpType type)
{
    assert(type >= 0 && type < 12 && "Invalid OpType_code");
    return OpType_strings[type];
}

void Parameters_destroy(Parameters *params)
{
    int i;
    if(params) {
        for(i = 0; i < params->i; i++) {
            // don't delete if it's listed in the module or function data
            free(params->args[i]);
        }

        free(params);
    }
}


Function *Function_find(Module *state, const char *name, int len)
{
    return Naming_search(state->functions, name, len);
}


FunctionParam *FunctionParam_create(FunctionParam *prev, Token *ident, Token *type)
{
    FunctionParam *p = calloc(1, sizeof(FunctionParam));
    assert(p && "Failed to allocate FunctionParam.");

    p->len = ident->len;
    p->name = ident->start;
    p->type = type->value;

    if(prev) {
        SGLIB_SORTED_LIST_ADD(FunctionParam, prev, p, FUNCTION_COMPARE, next);
        return prev;
    } else {
        return p;
    }
}


/* Returns a -1 when it doesn't find that parameter. */
int FunctionParam_index(Function *func, Token *ident)
{
    FunctionParam *param = NULL;
    FunctionParam elem = {.len = ident->len, .name = ident->start};
    assert(func && "Expected function, probably you're doing something outside a function definition.");

    SGLIB_SORTED_LIST_FIND_MEMBER(FunctionParam, func->params, &elem, FUNCTION_COMPARE, next, param);
 
    if(param) {
        return param->index;
    } else {
        return -1;
    }
}


void Function_destroy_labels(Function *func)
{
    Naming_destroy(func->labels, (Naming_destroy_cb)Label_destroy);

    func->labels = NULL;
}

static void data_destroy(Token *expr) {
    if(expr->id == TK_STR) {
        free((char *)expr->value);
    }
    free(expr);
}

void Function_destroy(Function *func)
{

    Function_destroy_labels(func);

    SGLIB_SORTED_LIST_MAP_ON_ELEMENTS(FunctionParam, func->params, i, next, free(i));

    Naming_destroy(func->data, (Naming_destroy_cb)data_destroy);

    free(func->name);
    free(func->code);
    free(func);
}


Token *Function_resolve_param(Module *state, Function *func, Token *ident)
{
    if(!func) return ident;  // skip any processing if not in a function

    unsigned long value = (unsigned long)FunctionParam_index(func, ident);
    if(value != -1) {
        // it's a function parameter
        ident->value = value;
    } else {
        return NULL;
    }

    return ident;
}
