#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "vm.h"


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

void cb(Result *r) {
    if(r && r->err) {
        printf("error: %s\n", r->err);
    }
}

int main(int argc, char **argv) {
    char *file_source = NULL;
    const char *src = NULL;

    if (argc > 1) {
        file_source = read_file(argv[1]);
        if (!file_source) return 1;
        src = file_source;
    } else {
        printf("no source\n");
        return 1;
    }

    size_t   size = 1024;
    uint8_t *code = malloc(size);
    memset(code, 0, size);

    assemble(src, code, size, cb);

    free(file_source);

    // if (!asm_result.ok) {
    //     fprintf(stderr, "assembler error");
    //     if (asm_result.line > 0) fprintf(stderr, " (line %d)", asm_result.line);
    //     fprintf(stderr, ": %s\n", asm_result.message);
    //     return 1;
    // }

    // return 0;

    Vm vm = { 0 };
    vm_init(&vm, code, size);
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
