#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instr.h"
#include "lexer.h"
#include "result.h"


struct Def {
    const char *name;
    size_t      len;
    uint16_t    val;
};

struct Link {
    const char *name;
    size_t      len;
    uint16_t    addr;
};

typedef enum : uint8_t {
    STATE_INSTR_OR_DEF,
    STATE_REG_L,
    STATE_REG_R,
    STATE_IMM,
    STATE_NEWLINE,
} State;

void next_state_of_layout(State *state, Layout layout) {
    switch(*state) {
    case STATE_INSTR_OR_DEF:
        if(layout_uses_reg_l(layout)) {
            *state = STATE_REG_L;
        }
        else if(layout_uses_reg_r(layout)) {
            *state = STATE_REG_R;
        }
        else if(layout_uses_imm(layout)) {
            *state = STATE_IMM;
        }
        else {
            *state = STATE_NEWLINE;
        }
        break;
    case STATE_REG_L:
        if(layout_uses_reg_r(layout)) {
            *state = STATE_REG_R;
        }
        else if(layout_uses_imm(layout)) {
            *state = STATE_IMM;
        }
        else {
            *state = STATE_NEWLINE;
        }
        break;
    case STATE_REG_R:
        if(layout_uses_imm(layout)) {
            *state = STATE_IMM;
        } else {
            *state = STATE_NEWLINE;
        }
        break;
    case STATE_IMM:
        *state = STATE_NEWLINE;
        break;
    case STATE_NEWLINE:
        *state = STATE_INSTR_OR_DEF;
        break;
    }
}

void assemble(const char *src, uint8_t *code, size_t cap, void cb(Result *r)) {
    const InstrInfo *info;
    Instr instr = { 0 };
    State state = STATE_INSTR_OR_DEF;
    size_t  ptr = 0;

    const char *imm_label     = NULL;
    size_t      imm_label_len = 0; 

    Result r = { 0 };

    const size_t n = 64;
    char scratch[n + 1];
    scratch[n] = '\0';

    size_t defs_count  = 0;
    size_t defs_cap    = 2048;
    struct Def *defs   = malloc(defs_cap * sizeof(struct Def));

    size_t links_count = 0;
    size_t links_cap   = 2048;
    struct Link *links = malloc(links_cap * sizeof(struct Link));

    while(*src) {
        Label label;
        Token tk;

        size_t ate = parse_token(src, &tk, &label, &r);

        if(r.err) {
            cb(&r);
            result_deinit(&r);
        }

        switch(state) {
        case STATE_INSTR_OR_DEF:
            switch(tk.kind) {
            case TOKEN_MNEMONIC:
                info = instr_info_of_opcode((uint8_t)tk.value);
                instr.opcode  = info->opcode;
                instr.layout  = info->layout;
                imm_label     = NULL;
                imm_label_len = 0;
                next_state_of_layout(&state, info->layout);
                break;
            case TOKEN_DEF:
                if(defs_count >= defs_cap) {
                    ERR(&r, "defs overflow, max=%d", defs_cap);
                    cb(&r);
                    result_deinit(&r);
                    goto defer;
                }

                defs[defs_count].name = label.start;
                defs[defs_count].len  = label.len;
                defs[defs_count].val  = ptr;
                defs_count++;

                state = STATE_INSTR_OR_DEF;
                break;
            case TOKEN_NEWLINE:
            case TOKEN_EOF:
                break;
            default:
                token_dump(scratch, n, tk, label);
                ERR(&r, "unexpected token: %s", scratch);
                cb(&r);
                result_deinit(&r);
            }
            break;

        case STATE_REG_L:
            if(tk.kind == TOKEN_REG) {
                instr.reg_l = tk.value & 0xf;
                next_state_of_layout(&state, info->layout);
            } else {
                token_dump(scratch, n, tk, label);
                ERR(&r, "expected reg l, found token: %s", scratch);
                cb(&r);
                result_deinit(&r);
                goto defer;
            }
            break;

        case STATE_REG_R:
            if(tk.kind == TOKEN_REG) {
                instr.reg_r = tk.value & 0xf;
                next_state_of_layout(&state, info->layout);
            } else {
                token_dump(scratch, n, tk, label);
                ERR(&r, "expected reg r, found token: %s", scratch);
                cb(&r);
                result_deinit(&r);
                goto defer;
            }
            break;

        case STATE_IMM:
            if(tk.kind == TOKEN_IMM) {
                if(layout_uses_imm8(info->layout) && tk.value > 0xff) {
                    ERR(&r, "imm to large 0x%x, max=0xff", tk.value);
                    cb(&r);
                    result_deinit(&r);
                }

                instr.imm = tk.value;
                next_state_of_layout(&state, info->layout);
            }
            else if(tk.kind == TOKEN_LABEL) {
                instr.imm = 62;
                next_state_of_layout(&state, info->layout);

                imm_label     = label.start;
                imm_label_len = label.len;
            }
            else {
                token_dump(scratch, n, tk, label);
                ERR(&r, "expected imm, found token: %s", scratch);
                cb(&r);
                result_deinit(&r);
                goto defer;
            }
            break;

        case STATE_NEWLINE:
            switch(tk.kind) {
            case TOKEN_NEWLINE:
            case TOKEN_EOF:
                uint32_t encoded = encode_instr(instr);
                size_t instr_size = size_of_layout(instr.layout);

                for(uint8_t i = 0; i < instr_size; i++) {
                    if(ptr >= cap) {
                        ERR(&r, "code buffer overflow, cap=%d", cap);
                        cb(&r);
                        result_deinit(&r);
                        goto defer;
                    }

                    uint8_t byte = (encoded >> (24 - i * 8)) & 0xff;
                    code[ptr] = byte;
                    ptr += 1;
                }

                if(!imm_label) {
                    state = STATE_INSTR_OR_DEF;
                    break;
                }

                size_t imm_addr = 0;
                switch(instr.layout) {
                case LAYOUT_REG_REG_IMM16:
                    imm_addr += 1;
                case LAYOUT_REG_IMM16:
                case LAYOUT_IMM16:
                    imm_addr += 1;

                    if(links_count >= links_cap) {
                        ERR(&r, "too many labels, max=%d", links_cap);
                        cb(&r);
                        result_deinit(&r);
                        goto defer;
                    }

                    links[links_count].name = imm_label;
                    links[links_count].len  = imm_label_len;
                    links[links_count].addr = ptr - instr_size + imm_addr;
                    links_count++;

                    break;
                case LAYOUT_REG_IMM8:
                    ERR(&r, "expected 8bit imm found label `%.*s`", imm_label_len, imm_label);
                    cb(&r);
                    result_deinit(&r);
                    break;
                case LAYOUT_NONE:
                case LAYOUT_REG_REG:
                case LAYOUT_REG:
                case LAYOUT_ILLEGAL:
                    break;
                }

                state = STATE_INSTR_OR_DEF;
                break;
            default:
                // state = STATE_INSTR_OR_DEF;
                token_dump(scratch, n, tk, label);
                ERR(&r, "expected newline, found: %s", scratch);
                cb(&r);
                result_deinit(&r);
                break;
            }
            break;
        }

        src += ate;
    }

    #if false
    for(size_t i = 0; i < defs_count; i++) {
        printf("%.*s: 0x%x\n", defs[i].len, defs[i].name, defs[i].val);
    }

    for(size_t i = 0; i < links_count; i++) {
        printf("link(%.*s) @ 0x%x\n", links[i].len, links[i].name, links[i].addr);
    }
    #endif

    for(size_t i = 0; i < links_count; i++) {
        bool linked = false;
        struct Link link = links[i];

        for(size_t j = 0; j < defs_count; j++) {
            struct Def def = defs[j];

            if(link.len == def.len && !strncmp(link.name, def.name, min(link.len, def.len))) {
                code[link.addr]     = (def.val >> 8) & 0xff;
                code[link.addr + 1] =  def.val       & 0xff;
                linked = true;
            }
        }

        if(!linked) {
            ERR(&r, "undefined symbol `%.*s`", link.len, link.name);
            cb(&r);
            result_deinit(&r);
        }
    }

defer:
    free(defs );
    free(links);
    return;
}
