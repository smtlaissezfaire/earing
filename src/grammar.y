// Zed A. Shaw -- A weird assembler. Copyright 2008. */

%include {
  #include <assert.h>
  #include <stdio.h> 
}
%token_destructor { free($$); }

%parse_accept {
}

%syntax_error {
    state->errors++;

    if(state->current) {
        printf("%s: In function '%.*s':\n", state->module_name, 
            state->current->len, state->current->name);
        state->current = NULL;
    }

    printf("%s:%d: error at %s token in [",
            state->module_name, state->curline, yyTokenName[yymajor]);
   
    int i;
    for(i=1; i<=yypParser->yyidx; i++)
      printf(" %s",yyTokenName[yypParser->yystack[i].major]);

    printf(" | %s ] unexpected '%.*s'\n",
            yyTokenName[yymajor],
            yymajor != 0 ? TOKEN->len : 7, 
            yymajor != 0 ? TOKEN->start : "$ (EOF)");
}

%stack_overflow {
    printf("Stack overflowed, you suck after line: %u.\n", state->curline);
}

%token_prefix TK_

%extra_argument { Module *state }
%token_type { Token* }

// type and destructor specifications
%type parameters { Parameters *}
%type args { Parameters * }

// the function stuff is all taken care of by the Module_destroy
%type function {Function *}
%type function_decl {Function *}
%type function_params { FunctionParam *}
// FunctionParams are freed by the function destructor
%type label {Label *}
// labels are freed by the function destructor
%type function_reference {Token *}

module ::= statements.

statements ::= statement. 
           statements ::= statements statement.

statement ::= directive EOL.
    statement ::= constant EOL.
    statement ::= function EOL.
    statement ::= EOL.

constant ::= IDENT(I) EQ expr(E). {
         Module_create_constant(state, I, E); free(I);
    }

parameters(P) ::= LPAREN args(A) RPAREN. { P = A; }
       parameters(P) ::= . { P = calloc(1,sizeof(Parameters)); } 

args(A) ::= args(B) COMMA expr(E). { B->args[B->i++] = E; A=B; }
     args(A) ::= expr(E). { A = calloc(1,sizeof(Parameters)); A->args[A->i++] = E;} 
     args(A) ::= . { A = calloc(1,sizeof(Parameters)); }

operation ::= OP(O) DOT TYPE(T) parameters(P).  
            { Call_operation(state, state->current, O, T, P); }

            operation ::= OP(O) parameters(P).   
            { Call_operation(state, state->current, O, NULL, P); }

expr(E) ::= HEX(I). { E = I; }
     expr(E) ::= FLOAT(I). { E = I; }
     expr(E) ::= INT(I). { E = I; }
     expr(E) ::= STR(I). { E = I; }
     expr(E) ::= CHR(I). { E = I; }
     expr(E) ::= REG(I). { E = I; }
     expr(E) ::= IDENT(I). { E = Module_resolve_data(state, I); }
     expr(E) ::= function_reference(I). { E = I; }
     expr(E) ::= function_reference(F) DOT IDENT(I).
        { E = Module_outside_function_data(state, F, I); }

     expr(E) ::= LABEL(I). 
        { E = Label_expression(state->current, I); }


function_reference(R) ::= IDENT(M) DOT IDENT(F). 
        { R = Module_resolve_function(state, M, F); }

directive ::= PERCENT IDENT(I) parameters(P).
          { Module_call_directive(state, I, P); }

function(F) ::= FUNCTION function_decl(D) block END(E).
    { F = Function_finalize(state, D, E); }

function_decl(F) ::= IDENT(I) LPAREN function_params(P) RPAREN COLON TYPE(T). 
        {
            F = Function_create(state, T->value, I->start, I->len, P); 
            free(I); free(T);
        }

function_params(P) ::= IDENT(I) COLON TYPE(T) . 
                { P = FunctionParam_create(NULL, I, T); free(I); free(T); }

                function_params(P) ::= function_params(N) COMMA IDENT(I) COLON TYPE(T). 
                {
                    P = FunctionParam_create(N, I, T);
                    free(I);  // these have to be here or double free
                    free(T);
                }
                function_params(P) ::= . { P = NULL; }

block ::= element EOL.
      block ::= block element EOL.

element ::= operation.
      element ::= directive. 
      element ::= LABEL(L). { Label_statement(state->current, L); free(L); }
      element ::= IDENT(I) EQ expr(E). {
         Module_create_function_constant(state, I, E); free(I);
      }
      element ::= .

// error recovery productions
statements ::= statements error statement.

