Custom **16-bit RISC Emulator**, written in the **C programming language**.

The project implements a custom variable-length instruction set architecture (ISA), provides an emulator and an assembler.

## Build

```make
CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -O2 -D_CRT_SECURE_NO_WARNINGS
```

```fish
make                           # alias for: make build

make build                     # complie and link the project
make run ARGS=counter.asm      # run
make clean                     # cleanup the target/ directory
```
