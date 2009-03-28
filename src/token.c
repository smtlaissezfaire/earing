#include "token.h"
#include <stdlib.h>
#include "grammar.h"
#include <assert.h>
#include <string.h>


Token *Token_create(int id, int len, const char *start, f_inst_call call)
{
    Token *tk = calloc(1,sizeof(Token)); 
    tk->id = id; 
    tk->len = len; 
    tk->start = start; 
    tk->call = call;
    return tk;
}


int Token_copy(Token *tk, char *into, int max, int what)
{
    const char *source = NULL;
    size_t length = 0;

    switch(what) {
        case TK_CHR:
            assert(tk->len >= 2 && "Invalid CHR token, must be at least 2 long.");
            source = tk->start+1;
            length = 1;  // CHR is always 1 in size for now
            break;
        default:
            // everything else we just copy the results
            source = tk->start;
            length = tk->len;
            break;
    }

    if(max <= tk->len) {
        return -1;
    } else {
        strncpy(into, source, length);
        into[length] = '\0';
        return length;
    }
}
