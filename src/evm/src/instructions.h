#pragma once
#include "evm.h"

class Thread;

namespace Instructions
{
    enum class OpCode : byte
    {
        NOOP,
        SYSCALL,

        //LOADS AND STORES
        PUSH,
        POP,
        SLOAD,
        SSTORE,

        //BINOPS
        IADD,
        ISUB,
        IMUL,
        IDIV,
        DADD,
        DSUB,
        DMUL,
        DDIV,
        EQ,
        NEQ,

        //Branching
        JUMP,
        JUMPZ,
        JUMPNZ,
        CALL,
        RET,
        RETV,

        //CONVERTERS
        I2D,
        D2I,

        _COUNT
    };

    enum class SysCallCode : byte
    {
        EXIT,
        PRINTC,
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
        case OpCode::I2D:
        case OpCode::D2I:
        case OpCode::POP:
        case OpCode::IADD:
        case OpCode::ISUB:
        case OpCode::IMUL:
        case OpCode::IDIV:
        case OpCode::DADD:
        case OpCode::DSUB:
        case OpCode::DMUL:
        case OpCode::DDIV:
        case OpCode::EQ:
        case OpCode::NEQ:
        case OpCode::RET:
        case OpCode::RETV:
            return OP_CODE_SIZE;
        case OpCode::PUSH:
        case OpCode::JUMP:
        case OpCode::JUMPNZ:
        case OpCode::JUMPZ:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::CALL:
            return OP_CODE_SIZE + WORD_SIZE + sizeof(uint32_t);
        case OpCode::SYSCALL:
            return OP_CODE_SIZE + SYSCALL_CODE_SIZE;
        case OpCode::SLOAD:
        case OpCode::SSTORE:
            return OP_CODE_SIZE + sizeof(int64_t);
        default:
            assert(false && "Case not handled");
        }

        return 0;
    }

    std::string ToString(const byte *_instr);
}