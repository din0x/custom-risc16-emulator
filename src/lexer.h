#ifndef LEXER_H
#define LEXER_H

#include "result.h"
#include <stdint.h>


typedef enum : uint8_t {
    TOKEN_MNEMONIC,
    TOKEN_REG,
    TOKEN_IMM16,
    TOKEN_LABEL,
    TOKEN_DEF,
    TOKEN_NEWLINE,
    TOKEN_INVALID,
    TOKEN_EOF,
} TokenKind;

typedef struct {
    const char  *start;
    size_t       len;
} Label;

typedef struct {
    uint16_t  value;
    TokenKind kind;
} Token;

typedef struct {
    Token   *buf;
    size_t   len;
} Tokens;

void token_dump(Token tk, Label label);

size_t parse_token(const char *src, Token *tk, Label *label, Result *r);

#endif
