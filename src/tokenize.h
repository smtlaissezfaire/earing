#define TK(I,C) (Token_create(I,te-ts,ts,C))

#define KW(T)  Parse(pParser, TK_##T, TK(TK_##T,0), state);
#define SYM(T)  Parse(pParser, TK_##T, TK(TK_##T,0), state);
#define IDENT() Parse(pParser, TK_IDENT, TK(TK_IDENT,0), state);
#define OP(T)  Parse(pParser, TK_OP, TK(TK_OP,inst_##T), state);
#define TYPE(T)  tk = TK(TK_TYPE,0); tk->value = OpType_##T; Parse(pParser, TK_TYPE, tk, state);
#define CHR()  tk = TK(TK_CHR,0); tk->value = (int)tk->start[1]; Parse(pParser, TK_CHR, tk, state);
#define STR()  tk = Token_create(TK_STR, te-ts-2, ts+1, NULL); tk->value = (unsigned long)tk->start; Parse(pParser, TK_STR, tk, state);
#define INT()  tk = TK(TK_INT,0); tk->value = atoi(tk->start); Parse(pParser, TK_INT, tk, state);
#define FLOAT()  tk = TK(TK_FLOAT,0); tk->value = atof(tk->start); Parse(pParser, TK_FLOAT, tk, state);
#define HEX()  tk = TK(TK_FLOAT,0); tk->value = (int)strtol(tk->start, NULL, 16); Parse(pParser, TK_HEX, tk, state);
#define REG(T)   tk = TK(TK_REG,0); tk->value = JIT_##T; Parse(pParser, TK_REG, tk, state);
#define LABEL() Parse(pParser, TK_LABEL, TK(TK_LABEL,0), state);

#define NO_LEAF(name) if(state->current_is_leaf)\
        die(state, "You cannot do " # name " in a function declared %%leaf.");
