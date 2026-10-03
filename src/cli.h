#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

typedef struct {
    const char *input;
    const char *bin;
    const char *output;
    bool        run;
    bool        help;
    bool        isa;
} Args;

void parse_args(Args *args, int argc, char **argv);

#endif
