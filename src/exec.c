#include <stdio.h>
#include "exec.h"
#include "instr.h"
#include "ram.h"

static void exec_two_reg(Vm *vm, const Instr *ins) {
    uint16_t *l = &vm->reg[ins->reg_l], *r = &vm->reg[ins->reg_r];

    switch (ins->opcode) {
    case OPCODE_LD:
        ram_read_16(&vm->ram, l, *r);
        break;
    case OPCODE_ST:
        ram_write_16(&vm->ram, *l, *r);
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
        if (*r == 0) { vm_fault(vm, "division by zero"); return; }
        *l = *l / *r;
        break;
    case OPCODE_MOD:
        if (*r == 0) { vm_fault(vm, "modulo by zero"); return; }
        *l = *l % *r;
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
    default:
        break;
    }
}

static void exec_branch(Vm *vm, const Instr *ins) {
    uint16_t l = vm->reg[ins->reg_l];
    uint16_t r = vm->reg[ins->reg_r];
    bool take = false;

    switch (ins->opcode) {
    case OPCODE_BREQ: take = l == r; break;
    case OPCODE_BRGT: take = l >  r; break;
    case OPCODE_BRLT: take = l <  r; break;
    case OPCODE_BRNE: take = l != r; break;
    default: break;
    }

    if (take) vm->reg[REG_PC] = ins->imm;
}

static void exec_trap(Vm *vm) {
    if (vm->trap == NULL) {
        vm_fault(vm, "unhandled trap");
        return;
    }
    (vm->trap)(vm);
}

void execute(Vm *vm, const Instr *ins) {
    switch (ins->opcode) {
    case OPCODE_HALT:  vm->exit = true; break;
    case OPCODE_RET:   vm_do_ret(vm); break;
    case OPCODE_CALL16: vm_do_call(vm, ins->imm); break;
    case OPCODE_DUMP:   vm_dbg_dump(vm); break;

    case OPCODE_MOV8:
    case OPCODE_MOV16:
        vm->reg[ins->reg_l] = ins->imm;
        break;

    case OPCODE_CALL:  vm_do_call(vm, vm->reg[ins->reg_l]); break;
    case OPCODE_TRAP:   exec_trap(vm); break;

    case OPCODE_LD: case OPCODE_ST:
    case OPCODE_ADD: case OPCODE_SUB: case OPCODE_MUL: case OPCODE_DIV: case OPCODE_MOD:
    case OPCODE_OR:  case OPCODE_XOR: case OPCODE_AND: case OPCODE_NAND:
        exec_two_reg(vm, ins);
        break;

    case OPCODE_BREQ: case OPCODE_BRGT: case OPCODE_BRLT: case OPCODE_BRNE:
        exec_branch(vm, ins);
        break;

    default:
        vm_fault(vm, "unknown opcode");
        printf("opcode was 0x%02x at pc=todo\n", ins->opcode);
        break;
    }
}
