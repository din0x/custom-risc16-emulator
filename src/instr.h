#ifndef INSTR_H
#define INSTR_H

#include <stdint.h>
#include <stdbool.h>
#include "vm.h"

enum layout : uint8_t {
    LAYOUT_NONE           = 0b00000000,
    LAYOUT_REG_REG        = 0b00001100,
    LAYOUT_IMM16          = 0b00000011,
    LAYOUT_REG_REG_IMM16  = 0b00001111,
    LAYOUT_REG            = 0b10000000,
    LAYOUT_REG_IMM8       = 0b10000001,
    LAYOUT_REG_IMM16      = 0b10000010,

    LAYOUT_ILLEGAL        = 0xfe,
};

enum opcode : uint8_t {
    OPCODE_RET            = 0x01,
    OPCODE_DUMP           = 0x02,
    OPCODE_TRAP           = 0x03,
    OPCODE_HALT           = 0x04,

    OPCODE_LD             = 0x10,
    OPCODE_ST             = 0x11,

    OPCODE_ADD            = 0x12,
    OPCODE_SUB            = 0x13,
    OPCODE_MUL            = 0x14,
    OPCODE_DIV            = 0x15,
    OPCODE_MOD            = 0x16,
    OPCODE_OR             = 0x17,
    OPCODE_XOR            = 0x18,
    OPCODE_AND            = 0x19,
    OPCODE_NAND           = 0x1a,

    OPCODE_BREQ           = 0x20,
    OPCODE_BRGT           = 0x21,
    OPCODE_BRLT           = 0x22,
    OPCODE_BRNE           = 0x23,

    OPCODE_CALL16         = 0x30,

    OPCODE_MOV8           = 0x80,
    OPCODE_MOV16          = 0x90,
    OPCODE_CALL           = 0xa0,
    OPCODE_PUSH16           = 0xb0,
    OPCODE_POP16            = 0xc0,
};

struct instr {
    enum layout layout;
    enum opcode opcode;
    enum reg    reg_l, reg_r;
    uint16_t    imm;
};

struct instr_info {
    const char  *mnemonic;
    enum opcode  opcode;
    enum layout  layout;
};

static const struct instr_info INSTR_INFOS[] = {
    { "ret",    OPCODE_RET,    LAYOUT_NONE          },
    { "dump",   OPCODE_DUMP,   LAYOUT_NONE          },
    { "trap",   OPCODE_TRAP,   LAYOUT_NONE          },
    { "halt",   OPCODE_HALT,   LAYOUT_NONE          },

    { "ld",     OPCODE_LD,     LAYOUT_REG_REG       },
    { "st",     OPCODE_ST,     LAYOUT_REG_REG       },
    { "add",    OPCODE_ADD,    LAYOUT_REG_REG       },
    { "sub",    OPCODE_SUB,    LAYOUT_REG_REG       },
    { "mul",    OPCODE_MUL,    LAYOUT_REG_REG       },
    { "div",    OPCODE_DIV,    LAYOUT_REG_REG       },
    { "mod",    OPCODE_MOD,    LAYOUT_REG_REG       },
    { "or",     OPCODE_OR,     LAYOUT_REG_REG       },
    { "xor",    OPCODE_XOR,    LAYOUT_REG_REG       },
    { "and",    OPCODE_AND,    LAYOUT_REG_REG       },
    { "nand",   OPCODE_NAND,   LAYOUT_REG_REG       },

    { "br",     OPCODE_BREQ,   LAYOUT_REG_REG_IMM16 },
    { "brgt",   OPCODE_BRGT,   LAYOUT_REG_REG_IMM16 },
    { "brlt",   OPCODE_BRLT,   LAYOUT_REG_REG_IMM16 },
    { "brne",   OPCODE_BRNE,   LAYOUT_REG_REG_IMM16 },
    { "calli",  OPCODE_CALL16, LAYOUT_IMM16         },

    { "mov8",   OPCODE_MOV8,   LAYOUT_REG_IMM8      },
    { "mov16",  OPCODE_MOV16,  LAYOUT_REG_IMM16     },
    { "call",   OPCODE_CALL,   LAYOUT_REG           },
    { "push16", OPCODE_PUSH16, LAYOUT_REG           },
    { "pop16",  OPCODE_POP16,  LAYOUT_REG           },
};

static const size_t INSTR_INFO_COUNT = sizeof(INSTR_INFOS) / sizeof(*INSTR_INFOS);

struct reg_info {
    uint8_t      encoding;
    const char  *name;
    const char  *role;
};

static const struct reg_info REG_INFOS[] = {
    { 0x0, NULL,    "General purpose" },
    { 0x1, NULL,    "General purpose" },
    { 0x2, NULL,    "General purpose" },
    { 0x3, NULL,    "General purpose" },
    { 0x4, NULL,    "General purpose" },
    { 0x5, NULL,    "General purpose" },
    { 0x6, NULL,    "General purpose" },
    { 0x7, NULL,    "General purpose" },
    { 0x8, NULL,    "General purpose" },
    { 0x9, NULL,    "General purpose" },
    { 0xa, NULL,    "General purpose" },
    { 0xb, NULL,    "Reserved"        },
    { 0xc, NULL,    "Reserved"        },
    { 0xd, "flags", "Flags register"  },
    { 0xe, "sp",    "Stack pointer"   },
    { 0xf, "pc",    "Program counter" },
};

static const size_t REG_INFOS_COUNT = sizeof(REG_INFOS) / sizeof(*REG_INFOS);

enum layout layout_of_opcode(enum opcode opcode);

bool is_opcode_packed(enum opcode opcode);
bool is_layout_packed(enum layout layout);

size_t size_of_layout(enum layout layout);

uint32_t encode_instr(struct instr  instr);
struct instr decode_instr(uint32_t raw);

void instr_dump(struct instr instr, const struct vm *vm);

bool layout_uses_reg_l(enum layout layout);
bool layout_uses_reg_r(enum layout layout);
bool layout_uses_imm  (enum layout layout);
bool layout_uses_imm8 (enum layout layout);
bool layout_uses_imm16(enum layout layout);

const struct instr_info *instr_info_of_opcode(enum opcode opcode);
const struct instr_info *instr_info_of_mnemonic(const char *mnemonic, size_t len);

#endif
