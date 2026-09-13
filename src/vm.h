#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include "ram.h"

typedef enum : uint8_t {
    REG_PC     = 0,
    REG_SP     = 1,
    REG_FLAGS  = 2,
} Reg;

typedef struct Vm {
    uint16_t reg[16];
    Ram ram;
    void (*trap)(struct Vm *vm);
    bool exit;
    bool fault;
} Vm;

void vm_init(Vm *vm, uint8_t *ram_buf, uint16_t ram_size);
void vm_step(Vm *vm);
void vm_run(Vm *vm);
void vm_do_call(Vm *vm, uint16_t target);
void vm_do_ret(Vm *vm);
void vm_fault(Vm *vm, const char *msg);
void vm_dbg_dump(Vm *vm);

#endif
