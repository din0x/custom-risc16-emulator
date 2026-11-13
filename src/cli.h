#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

struct args {
    const char *input;
    const char *bin;
    const char *output;
    bool        run;
    bool        help;
    bool        isa;
};

void parse_args(struct args *args, int argc, char **argv);

#endif
