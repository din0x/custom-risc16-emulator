#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    CLI_ANY,
    CLI_INPUT,
    CLI_OUTPUT,
    CLI_BIN,
} CliState;

void parse_args(Args *args, int argc, char **argv) {
    memset(args, 0, sizeof(*args));
    CliState cli = CLI_ANY;

    for(int i = 0; i < argc; i++) {
        if(!i) {
            continue;
        }

        const char *arg = argv[i];

        switch(cli) {
        case CLI_ANY:
            if(!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
                args->help = true;
            }
            else if(!strcmp(arg, "--isa")) {
                args->isa = true;
            }
            else if(!strcmp(arg, "-i")) {
                cli = CLI_INPUT;
            }
            else if(!strcmp(arg, "-o")) {
                cli = CLI_OUTPUT;
            }
            else if(!strcmp(arg, "-b")) {
                cli = CLI_BIN;
            }
            else if(!strcmp(arg, "-r"))  {
                args->run = true;
            }
            else {
                printf("unexpected argument: %s\n", arg);
                exit(1);
            }
            break;
        case CLI_INPUT:
            args->input = arg;
            cli = CLI_ANY;
            break;
        case CLI_OUTPUT:
            args->output = arg;
            cli = CLI_ANY;
            break;
        case CLI_BIN:
            args->bin = arg;
            cli = CLI_ANY;
            break;
        }
    }
}
