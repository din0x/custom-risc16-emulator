#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include "result.h"

typedef struct {
    uint8_t *buf;
    uint16_t size;
} Ram;

Result ram_read_8   (const Ram *ram, uint8_t  *dst, uint16_t addr);
Result ram_read_16  (const Ram *ram, uint16_t *dst, uint16_t addr);
Result ram_write_8  (Ram *ram, uint16_t addr, uint8_t   val);
Result ram_write_16 (Ram *ram, uint16_t addr, uint16_t  val);

// uint8_t  ram_read8(Ram *ram, uint16_t addr);
// uint16_t ram_read16(Ram *ram, uint16_t addr);

// void ram_write8(Ram *ram, uint16_t addr, uint8_t val);
// void ram_write16(Ram *ram, uint16_t addr, uint16_t val);

#endif
