#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include "ram.h"
#include "result.h"

enum reg : uint8_t {
    REG_0      = 0x0,
    REG_1      = 0x1,
    REG_2      = 0x2,
    REG_3      = 0x3,
    REG_4      = 0x4,
    REG_5      = 0x5,
    REG_6      = 0x6,
    REG_7      = 0x7,
    REG_8      = 0x8,
    REG_9      = 0x9,
    REG_10     = 0xa,

    /* Reserved, may be assigned special meaning in the future */
    REG_11     = 0xb,
    REG_12     = 0xc,
    REG_13     = 0xd,
    REG_14     = 0xe,
    REG_15     = 0xf,

    /* Special registers */
    REG_FLAGS  = REG_13,
    REG_SP     = REG_14,
    REG_PC     = REG_15,
};

struct vm {
    void          (*trap)(struct vm *vm);
    uint16_t      reg[16];
    uint16_t      stack_bound;
    bool          exit;
    struct ram    ram;
    struct result fault;
};

void vm_init(struct vm *vm, uint8_t *ram_buf, uint16_t ram_size);
void vm_step(struct vm *vm);
void vm_run(struct vm *vm);
void vm_do_call(struct vm *vm, uint16_t target);
void vm_do_ret(struct vm *vm);
void vm_dbg_dump(struct vm *vm);

void vm_push_16(struct vm *vm, uint16_t val);
uint16_t vm_pop_16(struct vm *vm);

#endif
