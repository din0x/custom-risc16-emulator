#include <stdio.h>
#include <string.h>
#include "instr.h"
#include "exec.h"
#include "vm.h"

uint8_t fetch_next_byte(Vm *vm, uint16_t offset) {
    uint16_t pc = vm->reg[REG_PC];
    uint8_t byte = 0;
    ram_read_8(&vm->ram, &byte, pc + offset);
    return byte;
}

void vm_step(Vm *vm) {
    uint8_t a = fetch_next_byte(vm, 0);
    uint8_t b = fetch_next_byte(vm, 1);
    uint8_t c = fetch_next_byte(vm, 2);
    uint8_t d = fetch_next_byte(vm, 3);
    uint32_t raw = (a << 24) | (b << 16) | (c << 8) | d;

    Instr instr = decode_instr(raw);

    // printf("0x%x ", vm->reg[REG_PC]);
    // instr_dump(instr, vm);
    // printf("\n");

    size_t size = size_of_layout(instr.layout);
    vm->reg[REG_PC] += size;

    exec_instr(vm, instr);
}

void vm_run(Vm *vm) {
    while (!vm->exit) {
        vm_step(vm);
    }
}

void vm_init(Vm *vm, uint8_t *ram_buf, uint16_t ram_size) {
    memset(vm, 0, sizeof(*vm));
    vm->ram.buf = ram_buf;
    vm->ram.size = ram_size;
    vm->reg[REG_SP] = ram_size;
}

void vm_do_call(Vm *vm, uint16_t target) {
    uint16_t sp = (uint16_t)(vm->reg[REG_SP] - 2);
    vm->reg[REG_SP] = sp;
    ram_write_16(&vm->ram, sp, vm->reg[REG_PC]);
    vm->reg[REG_PC] = target;
}

void vm_do_ret(Vm *vm) {
    uint16_t sp = vm->reg[REG_SP];
    ram_read_16(&vm->ram, &vm->reg[REG_PC], sp);
    vm->reg[REG_SP] = sp + 2; // TODO: stack overflow
}

void vm_fault(Vm *vm, const char *msg) {
    printf("fault: %s\n", msg);
    vm->fault = true;
    vm->exit = true;
}

void vm_dbg_dump(Vm *vm) {
    printf("-- dbg --\n");
    for (int i = 0; i < 16; i++) {
        printf("r%-2d=%5u (0x%04x)  ", i, vm->reg[i], vm->reg[i]);
        if (i % 4 == 3) printf("\n");
    }
}
