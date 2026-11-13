#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include "result.h"

struct ram {
    uint8_t *buf;
    uint16_t size;
};

struct result ram_read_8  (const struct ram *ram, uint8_t  *dst, uint16_t addr);
struct result ram_read_16 (const struct ram *ram, uint16_t *dst, uint16_t addr);

struct result ram_write_8 (struct ram *ram, uint16_t addr, uint8_t   val);
struct result ram_write_16(struct ram *ram, uint16_t addr, uint16_t  val);

#endif
