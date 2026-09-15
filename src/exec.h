#ifndef EXEC_H
#define EXEC_H

#include "vm.h"
#include "instr.h"

void exec_instr(Vm *vm, Instr instr);

#endif
