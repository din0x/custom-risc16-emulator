#ifndef LEXER_H
#define LEXER_H

#include "result.h"
#include <stdint.h>

enum token_kind : uint8_t {
    TOKEN_MNEMONIC,
    TOKEN_REG,
    TOKEN_IMM,
    TOKEN_LABEL,
    TOKEN_DEF,
    TOKEN_NEWLINE,
    TOKEN_INVALID,
    TOKEN_EOF,
};

struct label {
    const char  *start;
    size_t       len;
};

struct token {
    uint16_t        value;
    enum token_kind kind;
};

void token_dump(char *s, size_t n, struct token tk, struct label label);

size_t parse_token(const char *src, struct token *tk, struct label *label, struct result *r);

#endif
