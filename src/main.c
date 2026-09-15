#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "vm.h"

const size_t RAM_SIZE = 1024;

void trap_print_r3(Vm *vm) {
    printf("%d\n", vm->reg[REG_3]);
}

char *read_file(const char *path) {
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

    if (argc > 1) {
        file_source = read_file(argv[1]);
        if (!file_source) return 1;
        source = file_source;
    } else {
        printf("no source\n");
        return 1;
    }

    size_t   size = RAM_SIZE;
    uint8_t *code = (uint8_t*)malloc(size);
    memset(code, 0, size);

    uint16_t code_len = 0;
    AsmResult asm_result = assemble(source, code, size, &code_len);
    free(file_source);

    if (!asm_result.ok) {
        fprintf(stderr, "assembler error");
        if (asm_result.line > 0) fprintf(stderr, " (line %d)", asm_result.line);
        fprintf(stderr, ": %s\n", asm_result.message);
        return 1;
    }

    Vm vm = { 0 };
    vm_init(&vm, code, sizeof(code));
    vm.trap = trap_print_r3;

    vm_run(&vm);

    if (vm.fault) {
        fprintf(stderr, "execution halted due to fault\n");

        free(code);
        return 1;
    }

    free(code);
    return 0;
}
