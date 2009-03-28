#include "module.h"

Label *Label_find(Function *func, Token *tk)
{
    return Naming_search(func->labels, tk->start, tk->len);
}


Token *Label_expression(Function *func, Token *tk)
{
    Label *label = Label_find(func, tk);

    if(!label) {
        // forward label expression, create but not realized
        label = Label_create_and_add(func, tk, 0);
    } else {
        if(label->realized) {
            // backward label expression
            tk->value = (unsigned long)label->ref;
        } else {
            // forward label expression that's being used again
            // do nothing with it, Call_operation will handle the gore
        }
    }

    return tk;
}


void Label_destroy(Label *label)
{
    free(label);
}

