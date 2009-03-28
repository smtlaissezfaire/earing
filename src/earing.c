#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "directives.h"
#include "module.h"
#include "util.h"
#include "repl.h"
#include <string.h>
#include "dis.h"
#include <unistd.h>
#include "error.h"

Module *current_module = NULL;

void query_functions(Function *func, void *ignored)
{
    printf("%.*s: %zu bytes of code, %d params defined\n", 
            func->len, func->name, func->code_len, func->nparams);
}

void run_repl(Module *state, const char *in_file) {
    printf("EaRing.  Copyright 2008 Zed A. Shaw.\n");
    printf("Done compiling %s.  Enter ? to get the function list.\n", in_file);
    char *cmd = NULL;
    current_module = state;
    repl_init(NULL);

    while((cmd = repl_prompt())) {
        if(cmd[0] == '?') {
            Naming_traverse(state->functions, (Naming_traverse_cb)query_functions, NULL);
        } else {
            run_function(state, cmd);
        }
        free(cmd);
    }
}


int main(int argc, char *argv[])
{
    size_t length = 0;
    char *input = NULL;
    const char *in_file = NULL;
    Module *state = NULL;
    int opt;
    int disassemble = 0;
    const char *func = "main";
    int interactive = 0;
    int listing = 0;

    while((opt = getopt(argc, argv, "lhdf:i")) != -1) {
        switch(opt) {
            case 'd': disassemble = 1; break;
            case 'f': func = optarg; break;
            case 'i': interactive = 1; break;
            case 'l': listing = 1; break;
            case 'h': /// fall through!
            default:
                die(NULL, "USAGE: earing [-d | -i] [-f function] <file.asm>\n");
                return 1;
        }
    }

    if(optind >= argc) {
        die(NULL, "You have to give a file.  Use -h to see the usage.");
        return 1;
    }

    in_file = argv[optind];
    input = mmap_file(in_file, &length);

    state = Module_create(in_file, 1024);
    Module_register_default_directives(state);

    if(!Module_compile(state, input, length)) {
        die(state, "Parsing failed with %d errors.\n", state->errors);
        Module_destroy(state);
        munmap(input, length);
        return 1;
    } else {
        if(listing) {
            Naming_traverse(state->functions, (Naming_traverse_cb)query_functions, NULL);
        } else if(disassemble) {
            dis_functions(state);
        } else if(interactive) {
            // go into interactive mode with the repl
            run_repl(state, in_file);
        } else {
            // run the given function or the "main" default
            run_function(state, func);
        }
    }

    Module_destroy(state);
    munmap(input, length);
    return 0;
}
