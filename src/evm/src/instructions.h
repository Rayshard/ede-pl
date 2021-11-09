#pragma once
#include "evm.h"

class Thread;

namespace Instructions
{
    enum class OpCode : vm_byte
    {
        NOOP,
        SYSCALL,
        CONVERT,

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
        ADD,
        SUB,
        MUL,
        DIV,
        EQ,
        NEQ,

        //Branching
        JUMP,
        JUMPZ,
        JUMPNZ,
        CALL,
        RET,
        RETV,

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

    enum class DataType : vm_byte
    {
        I8, UI8,
        I16, UI16,
        I32, UI32,
        I64, UI64,
        F32, F64
    };

#define OP_CODE_SIZE sizeof(Instructions::OpCode)
#define SYSCALL_CODE_SIZE sizeof(Instructions::SysCallCode)
#define DATA_TYPE_SIZE sizeof(Instructions::DataType)

    typedef void (*ExecutionFunc)(Thread*);
    extern ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];

    void Init();

    constexpr vm_ui64 GetSize(OpCode _opCode)
    {
        switch (_opCode)
        {
        case OpCode::NOOP:
        case OpCode::POP:
        case OpCode::RET:
        case OpCode::RETV:
        return OP_CODE_SIZE;
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::EQ:
        case OpCode::NEQ:
        return OP_CODE_SIZE + DATA_TYPE_SIZE;
        case OpCode::PUSH:
        return OP_CODE_SIZE + WORD_SIZE;
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
        case OpCode::CONVERT:
        return OP_CODE_SIZE + DATA_TYPE_SIZE + DATA_TYPE_SIZE;
        default:
        assert(false && "Case not handled");
        }

        return 0;
    }

    std::string ToString(const vm_byte* _instr);
    void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent);

    namespace PUSH
    {
        Word GetValue(const vm_byte* _instr);
        void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent);
    }

    namespace SYSCALL
    {
        SysCallCode GetCode(const vm_byte* _instr);
        void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent);
    }
}