#pragma once
#include "evm.h"

class Thread;

namespace Instructions
{
    enum class OpCode : vm_byte
    {
        NOOP,
        SYSCALL,

        //LOADS AND STORES
        PUSH,
        POP,
        SLOAD,
        SSTORE,
        LLOAD,
        LSTORE,
        PLOAD,
        PSTORE,
        GLOAD,
        GSTORE,
        MLOAD,
        MSTORE,

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

    enum class SysCallCode : vm_byte
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

    constexpr vm_ui64 GetSize(OpCode _opCode)
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
            return OP_CODE_SIZE + VM_PTR_SIZE;
        case OpCode::CALL:
            return OP_CODE_SIZE + VM_PTR_SIZE + VM_UI32_SIZE;
        case OpCode::SYSCALL:
            return OP_CODE_SIZE + SYSCALL_CODE_SIZE;
        case OpCode::SLOAD:
        case OpCode::SSTORE:
        case OpCode::MLOAD:
        case OpCode::MSTORE:
        case OpCode::GLOAD:
        case OpCode::GSTORE:
            return OP_CODE_SIZE + VM_I64_SIZE;
        case OpCode::LLOAD:
        case OpCode::LSTORE:
        case OpCode::PLOAD:
        case OpCode::PSTORE:
            return OP_CODE_SIZE + VM_UI32_SIZE;
        default:
            assert(false && "Case not handled");
        }

        return 0;
    }

    std::string ToString(const vm_byte * _instr);
}