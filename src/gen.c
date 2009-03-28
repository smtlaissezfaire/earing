#include "module.h"
#include "grammar.h"
#include "sglib.h"
#include "error.h"

// because of the way GNU lightning is designed we have to keep ALL of the
// code generation tasks in the same file.  Since the function management is
// here we also have to include the code ops.c here.
#include "ops.c"

void Function_synthesize_param(FunctionParam *p)
{
    switch(p->type) {
        case OpType_i:
            p->index = jit_arg_i(); break;
        case OpType_ui:
            p->index = jit_arg_ui(); break;
        case OpType_l:
            p->index = jit_arg_l(); break;
        case OpType_ul:
            p->index = jit_arg_ul(); break;
        case OpType_p:
            p->index = jit_arg_p(); break;
        case OpType_f:
            p->index = jit_arg_f(); break;
        case OpType_d:
            p->index = jit_arg_d(); break;
        case OpType_c:
            p->index = jit_arg_c(); break;
        case OpType_uc:
            p->index = jit_arg_uc(); break;
        case OpType_s:
            p->index = jit_arg_s(); break;
        case OpType_us:
            p->index = jit_arg_us(); break;
        default:
            assert(!"Invalid type given for function parameter.");
    }
}

Function *Function_create(Module *state, OpType type, const char *name, int len, FunctionParam *params)
{
    if(Naming_search(state->functions, name, len)) {
        die(state, "%.*s is already defined, what you think this is Ruby?.", len, name);
    }

    Function *func = calloc(1,sizeof(Function));
    assert(func && "Failed to allocate the function.");
    func->code = calloc(state->max_code_size, sizeof(jit_insn));
    func->code_len = state->max_code_size;
    func->name = strndup(name, len);
    func->len = len;
    func->type = type;
    func->params = params;
    func->start = name;

    func->fp = jit_set_ip(func->code);

    SGLIB_SORTED_LIST_LEN(FunctionParam, func->params, next, func->nparams);

    if(state->current_is_leaf) {
        jit_leaf(func->nparams);
    } else {
        jit_prolog(func->nparams);
    }

    if(func->params) {
        SGLIB_LIST_MAP_ON_ELEMENTS(FunctionParam, func->params, p, next, 
                Function_synthesize_param(p);
        );
    }

    // now we just record ourselves in the state for further processing
    state->functions = Naming_insert(state->functions, func->name, func->len, func);
    state->current = func;

    return func;
}

Function *Function_finalize(Module *state, Function *func, Token *end)
{
    jit_ret();

    func->code_len = jit_get_ip().ptr - (char *)func->code;
    jit_flush_code(func->code, jit_get_ip().ptr);
    Function_destroy_labels(func);

    func->end = end->start + end->len;

    state->current = NULL;
    state->current_is_leaf = 0;

    free(end);
    return func;
}

void Label_patch_back_refs(Label *label)
{
    int i;
    for(i = 0; i < label->back_refs_i; i++) {
        jit_patch(label->back_refs[i]);
        label->back_refs[i] = NULL;
    }
    label->back_refs_i = 0;
}
Label *Label_create_and_add(Function *func, Token *tk, int realized)
{
    assert(tk && "Must give a tk.");
    assert(func && "Must have an active function.");

    Label *result = calloc(1, sizeof(Label));
    result->len = tk->len;
    result->name = tk->start;
    result->ref = jit_get_label();
    result->realized = realized;
    func->labels = Naming_insert(func->labels, result->name, result->len, result);

    return result;
}

void Label_statement(Function *func, Token *tk)
{
    Label *label = Label_find(func, tk);
    if(!label) {
        // forward label statement, create realized
        label = Label_create_and_add(func, tk, 1);
    } else {
        // backward label statement
        assert(!label->realized && "You've already used this label.");

        // patch the back references
        label->realized = 1;
        Label_patch_back_refs(label);
        label->ref = jit_get_label();
    }
}

static int Call_param_is(Parameters *params, int i, int tk) {
    return params->i > i && params->args[i]->id == tk;
}

void Label_add_back_ref(Label *label, jit_insn *ref)
{
    assert(label->back_refs_i < MAX_LABEL_BACK_REFS && "You exceeded the maximum label back references.");
    label->back_refs[label->back_refs_i++] = ref;
}

void Call_operation(Module *state, Function *func, Token *op, Token *type, Parameters *params)
{
    // TODO: adapt this to handle any parameter as a label
    Label *label = NULL;
    if(Call_param_is(params, 0, TK_LABEL)) {
        label = Label_find(func, params->args[0]);
        assert(label && "Label should be found.");

        if(!label->realized) {
            // adjust the value of this parameter to the jit_forward for the call
            params->args[0]->value = (unsigned long)jit_forward();
        } // otherwise value was set by Label_expression
    }

    assert(jit_get_ip().ptr - (char *)func->code < func->code_len && "You exceeded the space specified for function code blocks.");
    op->call(state, type, params);

    if(Call_param_is(params, 0, TK_LABEL) && !label->realized) {
        // we have to update backrefs here, and not in the label code
        // since the label is created outside the opcode run
        Label_add_back_ref(label, jit_get_label());
    }

    Parameters_destroy(params);
    if(op) free(op);
    if(op) free(type);
}

