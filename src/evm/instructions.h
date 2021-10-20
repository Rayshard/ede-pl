#pragma once

#define WORD_SIZE sizeof(unsigned long long)

enum class OpCode : unsigned char
{
    PUSH,
    ADDI,
    SUBI,
    MULI,
    DIVI,
    ADDD,
    SUBD,
    MULD,
    DIVD,
    POP,
    JUMP,
    EXIT,
    _COUNT
};

#define OPCODE_SIZE sizeof(OpCode)

unsigned long long InstructionSizes[] = {
    OPCODE_SIZE + WORD_SIZE, //PUSH
    OPCODE_SIZE,             //ADDI
    OPCODE_SIZE,             //SUBI
    OPCODE_SIZE,             //MULI
    OPCODE_SIZE,             //DIVI
    OPCODE_SIZE,             //ADDD
    OPCODE_SIZE,             //SUBD
    OPCODE_SIZE,             //MULD
    OPCODE_SIZE,             //DIVD
    OPCODE_SIZE,             //POP
    OPCODE_SIZE,             //JUMP
    OPCODE_SIZE,             //EXIT
};

static_assert(sizeof(InstructionSizes) / sizeof(InstructionSizes[0]) == (unsigned long long)OpCode::_COUNT);