#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "result.h"
#include <stdint.h>
#include <stdbool.h>


void assemble(const char *src, uint8_t *code, size_t cap, void cb(Result *r));

#endif
