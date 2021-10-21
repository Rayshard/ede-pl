#pragma once

#define OP_CODES PUSH, IADD, ISUB, IMUL, IDIV, DADD, DSUB, DMUL, DDIV, POP, JUMP, EXIT
#define INSTRUCTION_SIZES 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1
#define WORD_SIZE 8
#define OP_CODE_SIZE 1