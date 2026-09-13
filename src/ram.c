#include <stdint.h>
#include "ram.h"
#include "result.h"

Result ram_read_8(const Ram *ram, uint8_t *dst, uint16_t addr) {
    if(addr > ram->size) {
        ERR("read(%x): out of bounds, size=%x", addr, ram->size);
    }

    *dst = ram->buf[addr];
    OK
}

Result ram_read_16(const Ram *ram, uint16_t *dst, uint16_t addr) {
    uint8_t h = 0, l = 0;
    TRY(ram_read_8(ram, &h, addr    ));
    TRY(ram_read_8(ram, &l, addr + 1));
    *dst = (uint16_t)h << 8 | (uint16_t)l;
    OK
}

Result ram_write_8(Ram *ram, uint16_t addr, uint8_t val) {
    if(addr > ram->size) {
        ERR("write(%x): out of bounds, size=%x", addr, ram->size);
    }

    ram->buf[addr] = val;
    OK
}

Result ram_write_16(Ram *ram, uint16_t addr, uint16_t val) {
    uint8_t h = (val & 0xff00 >> 8);
    uint8_t l =  val & 0xff;

    TRY(ram_write_8(ram, addr,     h));
    TRY(ram_write_8(ram, addr + 1, l));

    OK
}
