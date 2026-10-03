#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "cli.h"
#include "instr.h"
#include "result.h"
#include "vm.h"


const char *HELP;

void trap_print_r3(Vm *vm) {
    printf("%d\n", vm->reg[REG_3]);
}

void cb(Result *r, bool *cx) {
    *cx = true;
    if(r && r->err) {
        fprintf(stderr, "error: %s\n", r->err);
    }
}

void print_instr_info(const InstrInfo *info) {
    char mnemonic[64];
    snprintf(mnemonic, sizeof(mnemonic), "%s,", info->mnemonic);
    printf("0x%02x, %-7s ", info->opcode, mnemonic);

    if(layout_uses_reg_l(info->layout)) {
        printf("l");
    }
    else {
        printf("-");
    }

    if(layout_uses_reg_r(info->layout)) {
        printf("r");
    }
    else {
        printf("-");
    }

    if(layout_uses_imm8(info->layout)) {
        printf("-8");
    }
    else if(layout_uses_imm16(info->layout)) {
        printf("16");
    }
    else {
        printf("--");
    }

    printf(",");

    char fill = ' ';
    char bits[33];
    memset(bits, fill, sizeof(bits));
    bits[32] = '\0';

    switch (info->layout) {
    case LAYOUT_NONE:
        strcpy(bits, "0kkkkkkk");
        break;
    case LAYOUT_REG_REG:
        strcpy(bits, "0kkkkkkkllllrrrr");
        break;
    case LAYOUT_IMM16:
        strcpy(bits, "0kkkkkkkjjjjjjjjiiiiiiii");
        break;
    case LAYOUT_REG_REG_IMM16:
        strcpy(bits, "0kkkkkkkllllrrrrjjjjjjjjiiiiiiii");
        break;
    case LAYOUT_REG:
        strcpy(bits, "1kkkllll");
        break;
    case LAYOUT_REG_IMM8:
        strcpy(bits, "1kkklllliiiiiiii");
        break;
    case LAYOUT_REG_IMM16:
        strcpy(bits, "1kkklllljjjjjjjjiiiiiiii");
        break;
    default:
        break;
    }

    for(size_t i = 0; i < 32; i++) {
        if(bits[i] == 'k') {
            bits[i] = (info->opcode & (0x80 >> i)) ? '1' : '0';
        }
        if(bits[i] == '\0') {
            bits[i] = fill;
        }
    }

    for(size_t i = 0; i < 32; i++) {
        if(i % 8 == 0) {
            printf(" ");
        }
        printf("%c", bits[i]);
    }

    printf("\n");
}

void print_reg_info(const RegInfo *info) {
    printf("r%d, ", info->encoding);
    if(info->encoding < 0xa) {
        printf(" ");
    }

    printf("0x%x, ", info->encoding);

    if(info->name) {
        size_t name_len = strlen(info->name);
        printf("%s, ", info->name);
        for(size_t i = 0; i < 5 - name_len; i++) {
            printf(" ");
        }
    } else {
        printf(",      ");
    }

    printf("%s\n", info->role);
}

int main(int argc, char **argv) {
    Args args;
    parse_args(&args, argc, argv);

    if(args.help) {
        printf("%s", HELP);
        return 0;
    }

    if(args.isa) {
        printf("Opcode, Mnemonic, Operands, Bits\n");

        for(size_t i = 0; i < INSTR_INFO_COUNT; i++) {
            const InstrInfo *info = &INSTR_INFOS[i];
            print_instr_info(info);
        }

        printf("\nRegister, Encoding, Name, Role\n");

        for(size_t i = 0; i < REG_INFOS_COUNT; i++) {
            const RegInfo *info = &REG_INFOS[i];
            print_reg_info(info);
        }

        return 0;
    }

    if((bool)args.input == (bool)args.bin) {
        fprintf(stderr, "-i xor -b flags can be used\n");
        return 1;
    }

    size_t   code_end;
    size_t   size = 1024;
    uint8_t *code = malloc(size);
    memset(code, 0, size);

    if(args.input) {
        FILE *file = fopen(args.input, "rb");
        if (!file) {
            fprintf(stderr, "could not open %s\n", args.input);
            return 1;
        }

        fseek(file, 0, SEEK_END);
        long tell = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint8_t *src = malloc((size_t)tell + 1);

        size_t read = fread(src, 1, (size_t)tell, file);
        src[read] = '\0';
        fclose(file);

        bool err = false;
        code_end = assemble((const char*)src, code, size, (void(*))cb, &err);

        if(err) {
            return 1;
        }

        free(src);
    }
    else if(args.bin) {
        FILE *file = fopen(args.bin, "rb");
        if(!file) {
            fprintf(stderr, "could not open %s\n", args.bin);
            return 1;
        }

        fseek(file, 0, SEEK_END);
        code_end = ftell(file);
        fseek(file, 0, SEEK_SET);
        fread(code, sizeof(uint8_t), size, file);
    }

    if(args.output) {
        FILE *file = fopen(args.output, "wb");
        if(!file) {
            fprintf(stderr, "could not open %s\n", args.output);
            return 1;
        }

        fwrite(code, sizeof(uint8_t), code_end, file);
        fclose(file);
    }

    if(args.run) {
        Vm vm = { 0 };
        vm_init(&vm, code, size);
        vm.trap        = trap_print_r3;
        vm.stack_bound = 512;

        vm_run(&vm);

        if(vm.fault.err) {
            fprintf(stderr, "fault: %s\n", vm.fault.err);

            result_deinit(&vm.fault);
            free(code);
            return 1;
        }
    }

    free(code);
    return 0;
}

const char *HELP =
    "Custom 16-bit variable-length RISC assembler and emulator\n"
    "\n"
    "Usage: risc16 [OPTIONS]\n"
    "\n"
    "Options:\n"
    "\n"
    "  -i, --input <PATH>   Assembly file input\n"
    "  -b, --bin <PATH>     Binary file input\n"
    "  -o, --output <PATH>  Binary file output\n"
    "  -r                   Run\n"
    "  --isa                Print ISA\n"
    "  -h, --help           Print help\n"
    "\n";
