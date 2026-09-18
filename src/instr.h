#ifndef INSTR_H
#define INSTR_H

#include <stdint.h>
#include <stdbool.h>
#include "vm.h"

typedef enum : uint8_t {
    LAYOUT_NONE           = 0b00000000,
    LAYOUT_REG_REG        = 0b00001100,
    LAYOUT_IMM16          = 0b00000011,
    LAYOUT_REG_REG_IMM16  = 0b00001111,
    LAYOUT_REG            = 0b10000000,
    LAYOUT_REG_IMM8       = 0b10000001,
    LAYOUT_REG_IMM16      = 0b10000010,

    LAYOUT_ILLEGAL        = 0xfe,
} Layout;

typedef enum : uint8_t {
    OPCODE_HALT           = 0x00,
    OPCODE_RET            = 0x01,
    OPCODE_DUMP           = 0x02,
    OPCODE_TRAP           = 0x03,

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

    OPCODE_COUNT
} Opcode;

typedef struct {
    Layout    layout;
    Opcode    opcode;
    uint8_t   reg_l, reg_r;
    uint16_t  imm;
} Instr;

Layout layout_of_opcode(Opcode opcode);

bool is_opcode_packed(Opcode opcode);
bool is_layout_packed(Layout layout);

size_t size_of_layout(Layout layout);

uint32_t encode_instr(Instr instr);
Instr decode_instr(uint32_t raw);

void instr_dump(Instr instr, const Vm *vm);

bool layout_uses_reg_l(Layout layout);
bool layout_uses_reg_r(Layout layout);
bool layout_uses_imm  (Layout layout);
bool layout_uses_imm8 (Layout layout);
bool layout_uses_imm16(Layout layout);

typedef struct InstrInfo {
    const char  *mnemonic;
    Opcode      opcode;
    Layout      layout;
} InstrInfo;

const InstrInfo *instr_info_of_opcode(Opcode opcode);
const InstrInfo *instr_info_of_mnemonic(const char *mnemonic, size_t len);

#endif
