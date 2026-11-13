#include <stdint.h>
#include "ram.h"
#include "result.h"


struct result ram_read_8(const struct ram *ram, uint8_t *dst, uint16_t addr) {
    if(addr >= ram->size) {
        struct result r = { 0 };
        ERR(&r, "read(0x%x): out of bounds, size=0x%x", addr, ram->size);
        return r;
    }

    *dst = ram->buf[addr];
    OK;
}

struct result ram_read_16(const struct ram *ram, uint16_t *dst, uint16_t addr) {
    uint8_t h = 0, l = 0;
    TRY(ram_read_8(ram, &h, addr    ));
    TRY(ram_read_8(ram, &l, addr + 1));
    *dst = (uint16_t)h << 8 | (uint16_t)l;
    OK;
}

struct result ram_write_8(struct ram *ram, uint16_t addr, uint8_t val) {
    if(addr >= ram->size) {
        struct result r = { 0 };
        ERR(&r, "write(0x%x): out of bounds, size=0x%x", addr, ram->size);
        return r;
    }

    ram->buf[addr] = val;
    OK;
}

struct result ram_write_16(struct ram *ram, uint16_t addr, uint16_t val) {
    uint8_t h = (val & 0xff00) >> 8;
    uint8_t l =  val & 0xff;

    TRY(ram_write_8(ram, addr,     h));
    TRY(ram_write_8(ram, addr + 1, l));

    OK;
}
