#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "exec.h"
#include "instr.h"
#include "ram.h"
#include "result.h"


void exec_trap(Vm *vm) {
    if (vm->trap == NULL) {
        ERR(&vm->fault, "unhandled trap");
        return;
    }
    (vm->trap)(vm);
}

void exec_instr(Vm *vm, Instr instr) {
    uint16_t *l     = &vm->reg[instr.reg_l];
    uint16_t *r     = &vm->reg[instr.reg_r];
    uint16_t imm    = instr.imm;

    bool     branch = false;

    switch (instr.opcode) {
    case OPCODE_HALT:
        vm->exit = true;
        break;
    case OPCODE_RET:
        vm_do_ret(vm);break;
    case OPCODE_CALL16:
        vm_do_call(vm, imm);
        break;
    case OPCODE_DUMP:
        vm_dbg_dump(vm);
        break;

    case OPCODE_MOV8:
    case OPCODE_MOV16:
        *l = imm;
        break;

    case OPCODE_CALL:
        vm_do_call(vm, *l);
        break;
    case OPCODE_TRAP:
        exec_trap(vm);
        break;

    case OPCODE_LD:
        vm->fault = ram_read_16(&vm->ram, l, *r);
        break;
    case OPCODE_ST:
        vm->fault = ram_write_16(&vm->ram, *l, *r);
        break;
    case OPCODE_ADD:
        *l += *r;
        break;
    case OPCODE_SUB:
        *l -= *r;
        break;
    case OPCODE_MUL:
        *l *= *r;
        break;
    case OPCODE_DIV:
        if(*r == 0) {
            ERR(&vm->fault, "division by zero ");
        } else {
            *l = *l / *r;
        }
        break;
    case OPCODE_MOD:
        if(*r == 0) {
            ERR(&vm->fault, "modulo by zero");
        } else {
            *l = *l % *r;
        }
        break;
    case OPCODE_OR:
        *l |= *r;
        break;
    case OPCODE_XOR:
        *l ^= *r;
        break;
    case OPCODE_AND:
        *l &= *r;
        break;
    case OPCODE_NAND:
        *l = ~(*l & *r);
        break;

    case OPCODE_BREQ:
        branch = *l == *r;
        break;
    case OPCODE_BRGT:
        branch = *l > *r;
        break;
    case OPCODE_BRLT:
        branch = *l < *r;
        break;
    case OPCODE_BRNE:
        branch = *l != *r;
        break;

    default:
        ERR(&vm->fault, "unknown opcode");
        printf("opcode was 0x%02x at pc=todo\n", instr.opcode);
        break;
    }

    if(vm->fault.err) {
        vm->exit = true;
    }

    if(branch) {
        vm->reg[REG_PC] = imm;
    }
}
