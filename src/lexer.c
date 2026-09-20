#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include "lexer.h"
#include "instr.h"
#include "result.h"


typedef struct {
    const char *start;
    size_t len;
} Span;

enum Parser {
    PARSER_ANY       = 1,
    PARSER_COMMENT   = 2,
    PARSER_IDENT     = 3,
    PARSER_FINISHED  = 4,
};

void token_dump(Token tk, Label label) {
    switch(tk.kind) {
    case TOKEN_MNEMONIC:
        const InstrInfo *info = instr_info_of_opcode((uint8_t)tk.value);
        printf("instr(%s)", info->mnemonic);
        break;
    case TOKEN_REG:
        printf("reg(r%d)", tk.value);
        break;
    case TOKEN_IMM16:
        printf("imm(%d)", tk.value);
        break;
    case TOKEN_LABEL:
        printf("label(%.*s)", label.len, label.start);
        break;
    case TOKEN_DEF:
        printf("def(%.*s)", label.len, label.start);
        break;
    case TOKEN_NEWLINE:
        printf("newline");
        break;
    case TOKEN_INVALID:
        printf("invalid");
        break;
    case TOKEN_EOF:
        printf("eof");
        break;
    default:
        printf("unhandled token kind %x\n", tk.kind);
        exit(1);
    }
}

size_t parse_token(const char *src, Token *tk, Label *label, Result *r) {
    label->start = 0;
    label->len   = 0;

    const char *start = src;

    enum Parser parser = PARSER_ANY;

    const char *instr_or_operand_start = 0;
    size_t      instr_or_operand_len   = 0;

    for(; *src; src++) {
        if(*src == '\n') {
            tk->kind = TOKEN_NEWLINE;
            tk->value = '\n';
            parser = PARSER_FINISHED;
            continue;
        }

        if(parser == PARSER_ANY) {
            if(*src == ';') {
                parser = PARSER_COMMENT;
            }
            else if(isalnum(*src) || *src == '_' || *src == ':') {
                parser = PARSER_IDENT;
                instr_or_operand_start = src;
                instr_or_operand_len   = 0;
            }
            else if(isspace(*src)) {
                continue;
            }
            else {
                ERR(r, "unexpected char `%c`", *src);
                tk->kind = TOKEN_INVALID;
                tk->value = 0;
                parser = PARSER_FINISHED;
                continue;
            }
        }

        switch(parser) {
        case PARSER_COMMENT:
            if(*src == '\n') {
                parser = PARSER_ANY;
                src--;
            }
            break;

        case PARSER_IDENT:
            if(!isalnum(*src) && *src != '_' && *src != ':') {
                size_t len = instr_or_operand_len;
                const char *start = instr_or_operand_start;

                const InstrInfo *info = instr_info_of_mnemonic(start, len);

                if(info) {
                    tk->kind  = TOKEN_MNEMONIC;
                    tk->value = info->opcode;
                }
                else if(len > 1 && len < 16 && *start == 'r') {
                    char buf[32];
                    memcpy(buf, start, len);
                    buf[len - 1] = '\0';

                    long n = strtol(start + 1, NULL, 10);

                    if(n < 0 || n > 15) {
                        ERR(r, "not a valid register `%.*s`", len, start);
                        tk->kind  = TOKEN_INVALID;
                        tk->value = 0;
                    } else {
                        tk->kind  = TOKEN_REG;
                        tk->value = (uint16_t)n;
                    }
                }
                else if(isalpha(*start)) {
                    label->start = start;
                    label->len = len;

                    if(*(src - 1) == ':') {
                        label->len--;
                        tk->kind = TOKEN_DEF;
                    } else {
                        tk->kind = TOKEN_LABEL;
                    }
                }
                else {
                    char *end;
                    unsigned long n = strtoul(start, &end, 10);

                    if(start == end) {
                        // printf("invalid ident\n");
                        tk->kind = TOKEN_INVALID;
                        tk->value = 0;
                    } else {
                        tk->kind = TOKEN_IMM16;
                        tk->value = (uint16_t)n;
                    }
                }

                // printf("instr_or_operand: %d, `%.*s`\n", len, len, start);
                src--;
                parser = PARSER_FINISHED;
                continue;
            }

            instr_or_operand_len++;
            break;

        case PARSER_FINISHED:
            return src - start;
            break;

        default:
            printf("unhandled parser state: %x\n", parser);
            exit(1);
            break;
        }
    }

    tk->kind = TOKEN_EOF;
    return src - start;
}
