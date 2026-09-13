#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>

#define ASM_ERR_MAX 256

typedef struct {
    bool ok;
    int  line;
    char message[ASM_ERR_MAX]; 
} AsmResult;

AsmResult assemble(const char *source, uint8_t *out, uint16_t out_cap, uint16_t *out_len);

#endif
