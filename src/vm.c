#include <stdio.h>
#include <string.h>
#include "instr.h"
#include "exec.h"
#include "result.h"
#include "vm.h"


uint8_t fetch_next_byte(Vm *vm, uint16_t offset) {
    uint16_t pc = vm->reg[REG_PC];
    uint8_t byte = 0;
    ram_read_8(&vm->ram, &byte, pc + offset);
    return byte;
}

void vm_step(Vm *vm) {
    uint32_t a = fetch_next_byte(vm, 0);
    uint32_t b = fetch_next_byte(vm, 1);
    uint32_t c = fetch_next_byte(vm, 2);
    uint32_t d = fetch_next_byte(vm, 3);
    uint32_t raw = (a << 24) | (b << 16) | (c << 8) | d;

    Instr instr = decode_instr(raw);
    if(instr.opcode == 0xff) {
        ERR(&vm->fault, "decode error");
        return;
    }

    #if false
    printf("0x%x ", vm->reg[REG_PC]);
    instr_dump(instr, vm);
    printf("\n");
    #endif

    size_t size = size_of_layout(instr.layout);
    vm->reg[REG_PC] += size;

    exec_instr(vm, instr);
}

void vm_run(Vm *vm) {
    while (!vm->exit) {
        vm_step(vm);

        if(vm->fault.err) {
            vm->exit = true;
        }
    }
}

void vm_init(Vm *vm, uint8_t *ram_buf, uint16_t ram_size) {
    memset(vm, 0, sizeof(*vm));
    result_init(&vm->fault);
    vm->stack_bound = 0xffff;
    vm->ram.buf = ram_buf;
    vm->ram.size = ram_size;
    vm->reg[REG_SP] = ram_size;
}

uint8_t vm_pop_8(Vm *vm) {
    uint16_t *sp = &vm->reg[REG_SP];

    if(*sp >= vm->ram.size) {
        ERR(&vm->fault, "stack underflow, sp=0x%40x, size=0x%40x", *sp, vm->ram.size);
        return 0;
    }

    uint8_t val;
    vm->fault = ram_read_8(&vm->ram, &val, *sp);

    if(vm->fault.err) {
        return 0;
    }

    (*sp)++;

    return val;
}

void vm_push_8(Vm *vm, uint8_t val) {
    uint16_t *sp = &vm->reg[REG_SP];

    if(*sp == 0 || *sp <= vm->stack_bound) {
        ERR(&vm->fault, "stack overflow, sp=0x%40x, bound=0x%40x", *sp, vm->stack_bound);
        return;
    }

    (*sp)--;
    vm->fault = ram_write_8(&vm->ram, *sp, val);
}

void vm_do_call(Vm *vm, uint16_t target) {
    uint16_t pc = vm->reg[REG_PC];

    vm_push_8(vm, (pc >> 8) & 0xff);
    if(vm->fault.err) {
        return;
    }

    vm_push_8(vm, pc & 0xff);
    if(vm->fault.err) {
        return;
    }

    vm->reg[REG_PC] = target;
}

void vm_do_ret(Vm *vm) {
    uint16_t l = vm_pop_8(vm);
    if(vm->fault.err) {
        return;
    }

    uint16_t h = vm_pop_8(vm);
    if(vm->fault.err) {
        return;
    }

    vm->reg[REG_PC] = (h << 8) | l;
}

void vm_dbg_dump(Vm *vm) {
    printf("-- dbg --\n");
    for (int i = 0; i < 16; i++) {
        printf("r%-2d=%5u (0x%04x)  ", i, vm->reg[i], vm->reg[i]);
        if (i % 4 == 3) {
            printf("\n");
        }
    }
}
