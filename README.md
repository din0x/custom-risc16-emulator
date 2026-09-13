Custom **16-bit RISC Emulator**, written in the **C programming language**.

The project implements a custom variable-length instruction set architecture (ISA), provides an emulator and an assembler.

## Build

By default the Makefile uses clang, to change that modify the `CC` in the Makefile.
To build the project run

```
make clean run ARGS="demo.asm"
```
