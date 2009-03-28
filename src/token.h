#ifndef token_h
#define token_h

struct Parameters;
struct Token;
struct Module;

/** 
 * This is attached to operation tokens and is called later to do the actual byte code
 * generation.
 */
typedef int (*f_inst_call)(struct Module *state, struct Token *type, struct Parameters *params);

/**
 * A Token holds almost all of the data, since an assembler is so simple
 * we can actually put the values inside the token.
 */
typedef struct Token {
    // all tokens have this
    int id;
    int len;
    const char *start;

    // different tokens have these set mutually exclusive
    f_inst_call call;
    // lightning apparently does lots of casting, so we just need the biggest numeric for storage
    unsigned long value;
} Token;

Token *Token_create(int id, int len, const char *start, f_inst_call call);

int Token_copy(Token *tk, char *into, int max, int what);

#endif
