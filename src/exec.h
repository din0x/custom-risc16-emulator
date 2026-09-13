#ifndef EXEC_H
#define EXEC_H

#include "vm.h"
#include "instr.h"

void execute(Vm *vm, const Instr *instr);

#endif
