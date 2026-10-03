Custom **16-bit RISC Emulator**, written in the **C programming language**.

The project implements a custom variable-length instruction set architecture (ISA), provides an emulator and an assembler.

## Build

```make
CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -O2 -D_CRT_SECURE_NO_WARNINGS
TARGET  := target/risc16
```

```fish
make                          # alias for: make build

make build                    # complie and link the project
make run                      # run
make clean                    # cleanup the target/ directory
```

## Usage

```fish
Usage: risc16 [OPTIONS]

Options:

  -i, --input <PATH>   Assembly file input
  -b, --bin <PATH>     Binary file input
  -o, --output <PATH>  Binary file output
  -r                   Run
  --isa                Print ISA
  -h, --help           Print help
```

## Examples

```fish
risc16 --isa                                    # print all instructions and layouts

risc16 -i examples/counter.asm -r               # assemble and run

risc16 -i examples/counter.asm -o counter.bin   # assemble to counter.bin
risc16 -b counter.bin -r                        # run raw binary program
```
