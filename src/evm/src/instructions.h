#pragma once
#include "evm.h"

class Thread;

namespace Instructions
{
    enum class OpCode : byte
    {
        NOOP,
        PUSH,
        POP,
        IADD,
        ISUB,
        IMUL,
        IDIV,
        DADD,
        DSUB,
        DMUL,
        DDIV,
        SLOAD,
        SSTORE,
        EQ,
        NEQ,
        JUMP,
        JUMPZ,
        JUMPNZ,
        SYSCALL,
        _COUNT
    };

    enum class SysCallCode : byte
    {
        EXIT,
        PRINTC,
        PRINTS,
        MALLOC,
        FREE,
        _COUNT
    };

#define OP_CODE_SIZE sizeof(Instructions::OpCode)
#define SYSCALL_CODE_SIZE sizeof(Instructions::SysCallCode)

    typedef void (*ExecutionFunc)(Thread *);
    extern ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];

    void Init();

    constexpr size_t GetSize(OpCode _opCode)
    {
        switch (_opCode)
        {
        case OpCode::NOOP:
            return OP_CODE_SIZE;
        case OpCode::PUSH:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::POP:
            return OP_CODE_SIZE;
        case OpCode::IADD:
            return OP_CODE_SIZE;
        case OpCode::ISUB:
            return OP_CODE_SIZE;
        case OpCode::IMUL:
            return OP_CODE_SIZE;
        case OpCode::IDIV:
            return OP_CODE_SIZE;
        case OpCode::DADD:
            return OP_CODE_SIZE;
        case OpCode::DSUB:
            return OP_CODE_SIZE;
        case OpCode::DMUL:
            return OP_CODE_SIZE;
        case OpCode::DDIV:
            return OP_CODE_SIZE;
        case OpCode::JUMP:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::JUMPNZ:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::JUMPZ:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::SYSCALL:
            return OP_CODE_SIZE + SYSCALL_CODE_SIZE;
        case OpCode::EQ:
            return OP_CODE_SIZE;
        case OpCode::NEQ:
            return OP_CODE_SIZE;
        case OpCode::SLOAD:
            return OP_CODE_SIZE + sizeof(int64_t);
        case OpCode::SSTORE:
            return OP_CODE_SIZE + sizeof(int64_t);
        default:
            assert(false && "Case not handled");
        }

        return 0;
    }
}