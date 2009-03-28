#ifndef module_h
#define module_h

#include <stdio.h>
#include <string.h>
#include "naming.h"
#include "token.h"
#include <lightning.h>

#define  MAX_LABEL_BACK_REFS 100

struct Module;
struct Parameters;


/**
 * These are used to determine the types of both operations and function returns.
 * For an operation it follows a '.' as in movi.ui(...).  For functions it follows
 * the function's name after a ':' as in function adder : int.  The lexer will actually
 * allow shorthand (i,ui,s,v) or long hand forms (int, uint, short, void) for
 * both functions and operations.
 */
typedef enum OpType {
    OpType_i,
    OpType_ui,
    OpType_l,
    OpType_ul,
    OpType_p,
    OpType_f,
    OpType_d,
    OpType_v,
    OpType_c,
    OpType_uc,
    OpType_s,
    OpType_us
} OpType;

const char *OpType_to_str(OpType type);

typedef struct Parameters {
    int i;
    Token *args[5];
} Parameters;


struct Function;

typedef struct Module {
    unsigned int curline;
    int errors;
    struct tnode *functions;
    struct Function *current;
    char *module_name;
    size_t max_code_size;
    tnode *directives;
    tnode *data;
    tnode *imports;
    tnode *libraries;
    int current_is_leaf;  // kind of a hack, need something else
} Module;

typedef void (*Module_directive_cb)(Module *state, Parameters *params);
typedef void(*Module_default_directives_cb)();

int Module_compile(Module *state, char *p, size_t length);

int Module_register_directive(Module *state, const char *name, int length, 
        Module_directive_cb call, Module_directive_cb destroy);

void Module_dump_directives(Module *state, FILE *out);

void Module_destroy(Module *state);

void Module_call_directive(Module *state, Token *ident, Parameters *params);

Token *Module_resolve_function(Module *state, Token *module, Token *ident);
Token *Module_resolve_data(Module *state, Token *ident);
Token *Module_outside_function_data(Module *state, Token *func_ref, Token *ident);

Module *Module_resolve_module(Module *state, Token *ident);

Module *Module_create(const char *name, size_t max_code_size);

void Module_create_constant(Module *state, Token *ident, Token *expr);
void Module_create_function_constant(Module *state, Token *ident, Token *expr);

typedef struct Directive {
    int len;
    const char *name;
    Module_directive_cb call;
    Module_directive_cb destroy;
} Directive;

typedef struct FunctionParam {
    int index;
    int len;
    OpType type;
    const char *name;
    struct FunctionParam *next;
} FunctionParam;

typedef struct Function {
    jit_insn *code;
    size_t code_len;
    jit_code fp;
    OpType type;
    char *name;
    int len;
    FunctionParam *params;
    int nparams;

    tnode *data;
    tnode *labels;

    const char *start;
    const char *end;
} Function;

typedef struct Label {
    jit_insn *ref;
    int len;
    const char *name;
    int realized;
    jit_insn *back_refs[MAX_LABEL_BACK_REFS];
    int back_refs_i;
} Label;

void Parameters_destroy(Parameters *params);

Function *Function_create(Module *state, OpType type, const char *name, int len, FunctionParam *params);
Function *Function_find(Module *state, const char *name, int len);
Function *Function_finalize(Module *state, Function *func, Token *end);
Token *Function_resolve_param(Module *state, Function *func, Token *ident);
FunctionParam *FunctionParam_create(FunctionParam *prev, Token *ident, Token *type);
int FunctionParam_index(Function *func, Token *ident);
void Function_destroy(Function *func);
void Function_destroy_labels(Function *func);


Label *Label_find_or_create(Function *func, Token *ident);
void Label_destroy(Label *label);
 
void Call_operation(Module *state, Function *func, Token *op, Token *type, Parameters *params);
Token *Label_expression(Function *func, Token *tk);
void Label_statement(Function *func, Token *tk);
Label *Label_find(Function *func, Token *tk);
Label *Label_create_and_add(Function *func, Token *tk, int realized);

char *strndup(const char *str, size_t len);
#endif


