#ifndef EXEC_H
#define EXEC_H

#include "vm.h"
#include "instr.h"

void exec_instr(struct vm *vm, struct instr instr);

#endif
