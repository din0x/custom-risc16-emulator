#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "vm.h"
#include "instr.h"

#define RAM_SIZE 1024

/* syscall 0: prints R3 as a signed decimal number */
static void sys_print_int(Vm *vm) {
    printf("%d\n", (int)(uint16_t)vm->reg[3]);
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "could not open '%s'\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t read = fread(buf, 1, (size_t)size, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    char *file_source = NULL;
    const char *source = NULL;
    // const char *source = DEMO_SOURCE;
    // printf("%s", DEMO_SOURCE);

    if (argc > 1) {
        file_source = read_file(argv[1]);
        if (!file_source) return 1;
        source = file_source;
    } else {
        printf("no source\n");
        return 1;
    }

    uint8_t code[RAM_SIZE];
    memset(code, 0, sizeof(code));
    uint16_t code_len = 0;

    AsmResult asm_result = assemble(source, code, sizeof(code), &code_len);
    free(file_source);

    if (!asm_result.ok) {
        fprintf(stderr, "assembler error");
        if (asm_result.line > 0) fprintf(stderr, " (line %d)", asm_result.line);
        fprintf(stderr, ": %s\n", asm_result.message);
        return 1;
    }

    for (size_t pc = 0; pc < 32; ) {
        uint8_t a = code[pc];
        uint8_t b = code[pc + 1];
        uint8_t c = code[pc + 2];
        uint8_t d = code[pc + 3];

        // printf("%x %x %x %x\n", a, b, c, d);

        uint32_t raw = (a << 24) | (b << 16) | (c << 8) | d;
        Instr instr = decode_instr(raw);
        size_t size = size_of_layout(instr.layout);

        // const InstrInfo *info = instr_info_of_opcode(instr.opcode);

        // printf("opcode(%x) layout(%x) size(%d)\n", instr.opcode, instr.opcode, instr.layout, size);

        // printf("%x ", pc);
        // instr_dump(instr);
        // printf("\n");

        if (size == 0) {
            printf("size cannot be zero\n");
            break;
        }
        pc += size;
    }

    Vm vm;
    vm_init(&vm, code, sizeof(code));
    vm.trap = sys_print_int;

    vm_run(&vm);

    if (vm.fault) {
        fprintf(stderr, "execution halted due to fault\n");
        return 1;
    }
    return 0;
}
