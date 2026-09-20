#include <stdio.h>
#include <string.h>
#include "instr.h"
#include "vm.h"


Layout layout_of_opcode(Opcode opcode) {
    switch(opcode) {
    case OPCODE_HALT:
    case OPCODE_RET:
    case OPCODE_DUMP:
    case OPCODE_TRAP:
        return LAYOUT_NONE;

    case OPCODE_LD:
    case OPCODE_ST:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_MOD:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_AND:
    case OPCODE_NAND:
        return LAYOUT_REG_REG;

    case OPCODE_MOV8:
        return LAYOUT_REG_IMM8;

    case OPCODE_MOV16:
        return LAYOUT_REG_IMM16;

    case OPCODE_CALL:
        return LAYOUT_REG;

    case OPCODE_CALL16:
        return LAYOUT_IMM16;

    case OPCODE_BREQ:
    case OPCODE_BRGT:
    case OPCODE_BRLT:
    case OPCODE_BRNE:
        return LAYOUT_REG_REG_IMM16;

    default:
        printf("illegal opcode 0x%x\n", opcode);
        return LAYOUT_ILLEGAL;
    }
}

size_t size_of_layout(Layout layout) {
    switch(layout) {
    case LAYOUT_NONE:
        return 1;
    case LAYOUT_REG:
        return 1;
    case LAYOUT_REG_REG:
        return 2;
    case LAYOUT_REG_IMM8:
        return 2;
    case LAYOUT_IMM16:
        return 3;
    case LAYOUT_REG_IMM16:
        return 3;
    case LAYOUT_REG_REG_IMM16:
        return 4;
    default:
        return 0;
    }
}

uint32_t encode_instr(Instr instr) {
    if(is_opcode_packed(instr.opcode) != is_layout_packed(instr.layout)) {
        printf("encode opcode and layout mismatch\n");
        return 0xffffffff;
    }

    uint32_t opcode = instr.opcode;
    uint32_t reg_reg = (instr.reg_l << 4) | instr.reg_r;
    uint32_t imm8 = instr.imm & 0xff;
    uint32_t imm16 = instr.imm;

    switch(instr.layout) {
    case LAYOUT_NONE:
        return opcode << 24;
    case LAYOUT_REG_REG:
        return (opcode << 24) | (reg_reg << 16);
    case LAYOUT_IMM16:
        return (opcode << 24) | (imm16 << 8);
    case LAYOUT_REG_REG_IMM16:
        return (opcode << 24) | (reg_reg << 16) | imm16;
    case LAYOUT_REG:
        return (opcode << 24) | (instr.reg_l << 24);
    case LAYOUT_REG_IMM8:
        return (opcode << 24) | (instr.reg_l << 24) | (imm8 << 16);
    case LAYOUT_REG_IMM16:
        return (opcode << 24) | (instr.reg_l << 24) | (imm16 << 8);
    default:
        return 0xffffffff;
    }
}

Opcode opcode_from_byte(uint8_t opcode) {
    if(is_opcode_packed(opcode)) {
        opcode &= 0xf0;
    }

    return opcode;
}

Instr decode_instr(uint32_t raw) {
    Instr instr = { 0 };

    uint8_t opcode = opcode_from_byte((raw >> 24) & 0xff);

    Layout layout = layout_of_opcode(opcode);

    if(is_opcode_packed(opcode) != is_layout_packed(layout)) {
        printf("decode opcode(%x) and layout(%x) mismatch\n", opcode, layout);
        instr.opcode = 0xff;
        return instr;
    }

    instr.layout = layout;
    instr.opcode = opcode;

    switch(instr.layout) {
    case LAYOUT_NONE:
        break;
    case LAYOUT_REG_REG:
        instr.reg_l = (raw >> 20) & 0xf;
        instr.reg_r = (raw >> 16) & 0xf;
        break;
    case LAYOUT_IMM16:
        instr.imm = (raw >> 8) & 0xffff;
        break;
    case LAYOUT_REG_REG_IMM16:
        instr.reg_l = (raw >> 20) & 0xf;
        instr.reg_r = (raw >> 16) & 0xf;
        instr.imm = raw & 0xffff;
        break;
    case LAYOUT_REG:
        instr.reg_l = (raw >> 24) & 0xf;
        break;
    case LAYOUT_REG_IMM8:
        instr.reg_l = (raw >> 24) & 0xf;
        instr.imm = (raw >> 16) & 0xff;
        break;
    case LAYOUT_REG_IMM16:
        instr.reg_l = (raw >> 24) & 0xf;
        instr.imm = (raw >> 8) & 0xffff;
        break;
    default:
        instr.layout = LAYOUT_ILLEGAL;
        break;
    }

    return instr;
}

bool is_opcode_packed(Opcode opcode) {
    return (opcode & 0x80) != 0;
}

bool is_layout_packed(Layout layout) {
    return (layout & 0x80) != 0;
}

void instr_dump(Instr instr, const Vm *vm) {
    const InstrInfo *info = instr_info_of_opcode(instr.opcode);

    printf("%s", info->mnemonic);
    if(layout_uses_reg_l(info->layout)) {
        printf(" r%d", instr.reg_l);
        if(vm) {
            printf("(%d)", vm->reg[instr.reg_l]);
        }
    }

    if(layout_uses_reg_r(info->layout)) {
        printf(" r%d", instr.reg_r);
        if(vm) {
            printf("(%d)", vm->reg[instr.reg_r]);
        }
    }

    if(layout_uses_imm(info->layout)) {
        printf(" %d", instr.imm);
    }
}

static const InstrInfo INSTR_INFOS[] = {
    { "halt",  OPCODE_HALT, LAYOUT_NONE  },
    { "ret",   OPCODE_RET, LAYOUT_NONE   },
    { "dump",  OPCODE_DUMP, LAYOUT_NONE  },
    { "trap",   OPCODE_TRAP, LAYOUT_NONE  },

    { "ld",    OPCODE_LD, LAYOUT_REG_REG    },
    { "st",    OPCODE_ST, LAYOUT_REG_REG    },
    { "add",   OPCODE_ADD, LAYOUT_REG_REG   },
    { "sub",   OPCODE_SUB, LAYOUT_REG_REG   },
    { "mul",   OPCODE_MUL, LAYOUT_REG_REG   },
    { "div",   OPCODE_DIV, LAYOUT_REG_REG   },
    { "mod",   OPCODE_MOD, LAYOUT_REG_REG   },
    { "or",    OPCODE_OR, LAYOUT_REG_REG    },
    { "xor",   OPCODE_XOR, LAYOUT_REG_REG   },
    { "and",   OPCODE_AND, LAYOUT_REG_REG   },
    { "nand",  OPCODE_NAND,  LAYOUT_REG_REG  },

    { "br",    OPCODE_BREQ, LAYOUT_REG_REG_IMM16  },
    { "brgt",  OPCODE_BRGT, LAYOUT_REG_REG_IMM16  },
    { "br>",   OPCODE_BRGT, LAYOUT_REG_REG_IMM16  },
    { "brlt",  OPCODE_BRLT, LAYOUT_REG_REG_IMM16  },
    { "br<",   OPCODE_BRLT, LAYOUT_REG_REG_IMM16  },
    { "brne",  OPCODE_BRNE, LAYOUT_REG_REG_IMM16  },
    { "br<>",  OPCODE_BRNE, LAYOUT_REG_REG_IMM16  },

    { "mov8",  OPCODE_MOV8, LAYOUT_REG_IMM8  },
    { "mov16", OPCODE_MOV16, LAYOUT_REG_IMM16 },
    { "call",  OPCODE_CALL, LAYOUT_REG  },
    { "calli",  OPCODE_CALL16, LAYOUT_IMM16 },
};

const size_t INSTR_INFO_COUNT = sizeof INSTR_INFOS / sizeof *INSTR_INFOS;

const InstrInfo *instr_info_of_opcode(Opcode opcode) {
    for (size_t i = 0; i < INSTR_INFO_COUNT; i++) {
        if (INSTR_INFOS[i].opcode == opcode) {
            return &INSTR_INFOS[i];
        }
    }

    return NULL;
}

const InstrInfo *instr_info_of_mnemonic(const char *mnemonic, size_t len) {
    for (size_t i = 0; i < INSTR_INFO_COUNT; i++) {
        if (!strncmp(INSTR_INFOS[i].mnemonic, mnemonic, len)) {
            return &INSTR_INFOS[i];
        }
    }

    return NULL;
}

bool layout_uses_reg_l(Layout layout) {
    return 
        layout == LAYOUT_REG ||
        layout == LAYOUT_REG_IMM16 ||
        layout == LAYOUT_REG_IMM8 ||
        layout == LAYOUT_REG_REG ||
        layout == LAYOUT_REG_REG_IMM16;
}

bool layout_uses_reg_r(Layout layout) {
    return layout == LAYOUT_REG_REG || layout == LAYOUT_REG_REG_IMM16;
}

bool layout_uses_imm(Layout layout) {
    return layout_uses_imm8(layout) || layout_uses_imm16(layout);
}

bool layout_uses_imm8(Layout layout) {
    return layout == LAYOUT_REG_IMM8;
}

bool layout_uses_imm16(Layout layout) {
    return layout == LAYOUT_REG_IMM16 || layout == LAYOUT_IMM16 || layout == LAYOUT_REG_REG_IMM16;
}
