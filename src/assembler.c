#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "instr.h"

#define MAX_LINES     4096
#define MAX_LABELS    1024
#define MAX_OPERANDS  4
#define TOKEN_LEN     64
#define LINE_BUF_LEN  256

typedef struct {
    char label[TOKEN_LEN];
    char mnemonic[TOKEN_LEN];
    char operands[MAX_OPERANDS][TOKEN_LEN];
    int  operand_count;
    int  line_no;
    uint16_t  address;
    uint8_t   size;
} ParsedLine;

typedef struct {
    char name[TOKEN_LEN];
    uint16_t  address;
} Label;

static void set_err(AsmResult *err, int line_no, const char *fmt, ...) {
    err->ok = false;
    err->line = line_no;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(err->message, sizeof(err->message), fmt, ap);
    va_end(ap);
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
    return s;
}

static bool is_valid_identifier(const char *s) {
    if (!*s) return false;
    if (!isalpha((unsigned char)s[0]) && s[0] != '_') return false;
    for (const char *p = s + 1; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_') return false;
    }
    return true;
}

static char *next_token(char **cursor, const char *delims) {
    char *s = *cursor;
    while (*s && strchr(delims, *s)) s++;
    if (*s == '\0') { *cursor = s; return NULL; }
    char *start = s;
    while (*s && !strchr(delims, *s)) s++;
    if (*s) { *s = '\0'; s++; }
    *cursor = s;
    return start;
}

static bool label_lookup(const Label *labels, int count, const char *name, uint16_t *addr) {
    for (int i = 0; i < count; i++) {
        if (strcmp(labels[i].name, name) == 0) { *addr = labels[i].address; return true; }
    }
    return false;
}

static bool parse_register(const char *tok, uint8_t *out) {
    const char *p = tok;
    if (*p == '$') p++;

    char low[TOKEN_LEN];
    size_t n = strlen(p);
    if (n >= sizeof(low)) return false;
    for (size_t i = 0; i < n; i++) low[i] = (char)tolower((unsigned char)p[i]);
    low[n] = '\0';

    if (strcmp(low, "pc") == 0)    { *out = REG_PC;    return true; }
    if (strcmp(low, "sp") == 0)    { *out = REG_SP;    return true; }
    if (strcmp(low, "flags") == 0) { *out = REG_FLAGS; return true; }

    if (low[0] == 'r' && n >= 2) {
        char *endptr;
        long v = strtol(low + 1, &endptr, 10);
        if (*endptr == '\0' && v >= 0 && v <= 15) { *out = (uint8_t)v; return true; }
    }
    return false;
}

static bool parse_number_literal(const char *tok, long *out) {
    if (!*tok) return false;
    char *endptr;
    long v = strtol(tok, &endptr, 0);
    if (endptr == tok || *endptr != '\0') return false;
    *out = v;
    return true;
}

static bool resolve_value16(const char *tok, const Label *labels, int label_count,
                             uint16_t *out, int line_no, AsmResult *err) {
    long v;
    if (parse_number_literal(tok, &v)) { *out = (uint16_t)v; return true; }

    uint16_t addr;
    if (label_lookup(labels, label_count, tok, &addr)) { *out = addr; return true; }

    set_err(err, line_no, "undefined label or invalid number '%s'", tok);
    return false;
}

static void tokenize_instruction(char *text, ParsedLine *pl) {
    char *cursor = text;
    char *tok = next_token(&cursor, " \t,");
    if (!tok) return;

    size_t n = strlen(tok);
    if (n >= TOKEN_LEN) n = TOKEN_LEN - 1;
    for (size_t i = 0; i < n; i++) pl->mnemonic[i] = (char)tolower((unsigned char)tok[i]);
    pl->mnemonic[n] = '\0';

    pl->operand_count = 0;
    while (pl->operand_count < MAX_OPERANDS &&
           (tok = next_token(&cursor, " \t,")) != NULL) {
        strncpy(pl->operands[pl->operand_count], tok, TOKEN_LEN - 1);
        pl->operands[pl->operand_count][TOKEN_LEN - 1] = '\0';
        pl->operand_count++;
    }
}

static bool split_line(const char *raw, ParsedLine *pl, int line_no, AsmResult *err) {
    memset(pl, 0, sizeof(*pl));
    pl->line_no = line_no;

    char buf[LINE_BUF_LEN];
    strncpy(buf, raw, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *comment = strpbrk(buf, ";#");
    if (comment) *comment = '\0';

    char *line = trim(buf);
    if (*line == '\0') return true;

    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        char *label_part = trim(line);
        if (!is_valid_identifier(label_part)) {
            set_err(err, line_no, "invalid label name '%s'", label_part);
            return false;
        }
        strncpy(pl->label, label_part, TOKEN_LEN - 1);
        line = trim(colon + 1);
        if (*line == '\0') return true;
    }

    tokenize_instruction(line, pl);
    return true;
}

static bool compute_size(ParsedLine *pl, AsmResult *err) {
    const InstrInfo *info = instr_info_of_mnemonic(pl->mnemonic, strlen(pl->mnemonic));

    if (!info) {
        set_err(err, pl->line_no,
                "unknown instruction '%s'", pl->mnemonic);
        return false;
    }

    pl->size = size_of_layout(info->layout);
    return true;
}

/* ---------- pass 2: emitting bytes ---------- */

static bool emit_byte(uint8_t **cursor, uint8_t *out_end, uint8_t val, int line_no, AsmResult *err) {
    if (*cursor >= out_end) {
        set_err(err, line_no, "program exceeds output buffer size");
        return false;
    }
    *(*cursor)++ = val;
    return true;
}

static bool emit_encoded_instr(
    uint8_t **cursor,
    uint8_t *out_end,
    uint32_t encoded,
    size_t size,
    int line_no,
    AsmResult *err)
{
    for (size_t i = 0; i < size; i++) {
        uint8_t byte = (uint8_t)((encoded >> (24 - i * 8)) & 0xff);

        if (!emit_byte(cursor, out_end, byte, line_no, err)) {
            return false;
        }
    }

    return true;
}

static bool emit_instruction(
    const ParsedLine *pl,
    const Label *labels,
    int label_count,
    uint8_t *out,
    uint8_t *out_end,
    AsmResult *err)
{
    const InstrInfo *info =
        instr_info_of_mnemonic(pl->mnemonic, strlen(pl->mnemonic));

    if (!info) {
        set_err(err, pl->line_no,
                "unknown instruction '%s'", pl->mnemonic);
        return false;
    }

    Instr instr = { 0 };

    instr.opcode = info->opcode;
    instr.layout = info->layout;

    int ln = pl->line_no;

    switch (instr.layout) {
    case LAYOUT_NONE:
        if (pl->operand_count != 0) {
            set_err(err, ln, "'%s' expects no operands, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }
        break;

    case LAYOUT_REG:
        if (pl->operand_count != 1) {
            set_err(err, ln, "'%s' expects 1 operand, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        if (!parse_register(pl->operands[0], (uint8_t*)&instr.reg_l)) {
            set_err(err, ln,
                    "expected a register operand, got '%s'",
                    pl->operands[0]);
            return false;
        }
        break;

    case LAYOUT_REG_REG:
        if (pl->operand_count != 2) {
            set_err(err, ln, "'%s' expects 2 operands, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        if (!parse_register(pl->operands[0], &instr.reg_l) ||
            !parse_register(pl->operands[1], &instr.reg_r)) {
            set_err(err, ln, "expected two register operands");
            return false;
        }
        break;

    case LAYOUT_IMM16:
        if (pl->operand_count != 1) {
            set_err(err, ln, "'%s' expects 1 operand, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        if (!resolve_value16(
                pl->operands[0],
                labels,
                label_count,
                &instr.imm,
                ln,
                err)) {
            return false;
        }
        break;

    case LAYOUT_REG_IMM8: {
        if (pl->operand_count != 2) {
            set_err(err, ln, "'%s' expects 2 operands, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        uint8_t reg;
        long value;

        if (!parse_register(pl->operands[0], &reg)) {
            set_err(err, ln,
                    "expected a register operand, got '%s'",
                    pl->operands[0]);
            return false;
        }

        if (!parse_number_literal(pl->operands[1], &value) ||
            value < 0 || value > 255) {
            set_err(err, ln,
                    "%s expects an 8-bit literal (0-255)",
                    pl->mnemonic);
            return false;
        }

        instr.reg_l = (uint8_t)reg;
        instr.imm = (uint16_t)value;
        break;
    }

    case LAYOUT_REG_IMM16:
        if (pl->operand_count != 2) {
            set_err(err, ln, "'%s' expects 2 operands, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        if (!parse_register(pl->operands[0],
                            (uint8_t*)&instr.reg_l)) {
            set_err(err, ln,
                    "expected a register operand, got '%s'",
                    pl->operands[0]);
            return false;
        }

        if (!resolve_value16(
                pl->operands[1],
                labels,
                label_count,
                &instr.imm,
                ln,
                err)) {
            return false;
        }
        break;

    case LAYOUT_REG_REG_IMM16:
        if (pl->operand_count != 3) {
            set_err(err, ln, "'%s' expects 3 operands, got %d",
                    pl->mnemonic, pl->operand_count);
            return false;
        }

        if (!parse_register(pl->operands[0],
                            (uint8_t *)&instr.reg_l) ||
            !parse_register(pl->operands[1],
                            (uint8_t *)&instr.reg_r)) {
            set_err(err, ln,
                    "expected two register operands before the branch target");
            return false;
        }

        if (!resolve_value16(
                pl->operands[2],
                labels,
                label_count,
                &instr.imm,
                ln,
                err)) {
            return false;
        }
        break;

    default:
        set_err(err, ln,
                "internal error: illegal layout for instruction '%s'",
                pl->mnemonic);
        return false;
    }

    uint32_t encoded = encode_instr(instr);

    if (encoded == 0xffffffffu) {
        set_err(err, ln,
                "internal error: failed to encode instruction '%s'",
                pl->mnemonic);
        return false;
    }

    size_t size = size_of_layout(instr.layout);

    uint8_t *cursor = out + pl->address;

    return emit_encoded_instr(
        &cursor,
        out_end,
        encoded,
        size,
        ln,
        err);
}

AsmResult assemble(const char *source, uint8_t *out, uint16_t out_cap, uint16_t *out_len) {
    AsmResult result;
    memset(&result, 0, sizeof(result));
    result.ok = true;

    ParsedLine *lines = calloc(MAX_LINES, sizeof(ParsedLine));
    Label *labels = calloc(MAX_LABELS, sizeof(Label));
    if (!lines || !labels) {
        set_err(&result, 0, "out of memory");
        free(lines);
        free(labels);
        return result;
    }

    int line_count = 0;
    int label_count = 0;

    const char *p = source;
    int line_no = 0;
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        char linebuf[LINE_BUF_LEN];
        size_t copy_len = len < sizeof(linebuf) - 1 ? len : sizeof(linebuf) - 1;
        memcpy(linebuf, p, copy_len);
        linebuf[copy_len] = '\0';
        if (copy_len > 0 && linebuf[copy_len - 1] == '\r') linebuf[copy_len - 1] = '\0';

        line_no++;
        if (line_count >= MAX_LINES) {
            set_err(&result, line_no, "program has too many lines (max %d)", MAX_LINES);
            goto done;
        }
        if (!split_line(linebuf, &lines[line_count], line_no, &result)) goto done;
        line_count++;

        p = nl ? nl + 1 : p + len;
    }

    /* --- pass 1: assign addresses, collect labels, compute sizes --- */
    {
        uint16_t addr = 0;
        for (int i = 0; i < line_count; i++) {
            lines[i].address = addr;

            if (lines[i].label[0]) {
                uint16_t dummy;
                if (label_lookup(labels, label_count, lines[i].label, &dummy)) {
                    set_err(&result, lines[i].line_no, "duplicate label '%s'", lines[i].label);
                    goto done;
                }
                if (label_count >= MAX_LABELS) {
                    set_err(&result, lines[i].line_no, "too many labels (max %d)", MAX_LABELS);
                    goto done;
                }
                strncpy(labels[label_count].name, lines[i].label, TOKEN_LEN - 1);
                labels[label_count].address = addr;
                label_count++;
            }

            if (lines[i].mnemonic[0]) {
                if (!compute_size(&lines[i], &result)) goto done;
                addr = (uint16_t)(addr + lines[i].size);
            }
        }

        if (addr > out_cap) {
            set_err(&result, 0, "assembled program (%u bytes) exceeds output buffer (%u bytes)",
                    addr, out_cap);
            goto done;
        }

        /* --- pass 2: resolve labels and emit real bytes --- */
        uint8_t *out_end = out + out_cap;
        for (int i = 0; i < line_count; i++) {
            if (!lines[i].mnemonic[0]) continue;
            if (!emit_instruction(&lines[i], labels, label_count, out, out_end, &result)) goto done;
        }

        *out_len = addr;
    }

done:
    free(lines);
    free(labels);
    return result;
}
