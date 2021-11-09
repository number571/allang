#ifndef ALL_KERNEL_H
#define ALL_KERNEL_H

#include <stdio.h>

// translate source file (input) into assembler listing file (output)
extern int all_compile(FILE *output, FILE *input);

#endif /* ALL_KERNEL_H */ 
