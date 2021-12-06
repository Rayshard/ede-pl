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

#pragma pack(push, 1)
#define OPERAND(type, name) type name;
#define INSTRUCTION(OPCODE, OPERANDS)                                                      \
    struct OPCODE                                                                          \
    {                                                                                      \
        const OpCode opcode = OpCode::OPCODE;                                              \
        OPERANDS                                                                           \
                                                                                           \
        static constexpr vm_ui64 GetSize() { return sizeof(OPCODE); }                      \
                                                                                           \
        static OPCODE* From(vm_byte* _instr)                                               \
        {                                                                                  \
            assert((OpCode)*_instr == OpCode::OPCODE && "Invalid instruction cast!");      \
            return (OPCODE*)_instr;                                                        \
        }                                                                                  \
                                                                                           \
        static const OPCODE* From(const vm_byte* _instr)                                   \
        {                                                                                  \
            assert((OpCode)*_instr == OpCode::OPCODE && "Invalid instruction cast!");      \
            return (const OPCODE*)_instr;                                                  \
        }                                                                                  \
    }

    INSTRUCTION(NOOP, );
    INSTRUCTION(POP, );
    INSTRUCTION(SYSCALL, OPERAND(SysCallCode, code));
    INSTRUCTION(CONVERT, OPERAND(DataType, from) OPERAND(DataType, to));
    INSTRUCTION(PUSH, OPERAND(Word, value));
    INSTRUCTION(SLOAD, OPERAND(vm_i64, offset));
    INSTRUCTION(SSTORE, OPERAND(vm_i64, offset));
    INSTRUCTION(MLOAD, OPERAND(vm_i64, offset));
    INSTRUCTION(MSTORE, OPERAND(vm_i64, offset));
    INSTRUCTION(LLOAD, OPERAND(vm_ui32, idx));
    INSTRUCTION(LSTORE, OPERAND(vm_ui32, idx));
    INSTRUCTION(PLOAD, OPERAND(vm_ui32, idx));
    INSTRUCTION(PSTORE, OPERAND(vm_ui32, idx));
    INSTRUCTION(ADD, OPERAND(DataType, type));
    INSTRUCTION(SUB, OPERAND(DataType, type));
    INSTRUCTION(MUL, OPERAND(DataType, type));
    INSTRUCTION(DIV, OPERAND(DataType, type));
    INSTRUCTION(EQ, OPERAND(DataType, type));
    INSTRUCTION(NEQ, OPERAND(DataType, type));
    INSTRUCTION(JUMP, OPERAND(vm_byte*, target));
    INSTRUCTION(JUMPZ, OPERAND(vm_byte*, target));
    INSTRUCTION(JUMPNZ, OPERAND(vm_byte*, target));
    INSTRUCTION(CALL, OPERAND(vm_byte*, target) OPERAND(vm_ui32, storage));
    INSTRUCTION(RET, );
    INSTRUCTION(RETV, );

#undef OPERAND
#undef INSTRUCTION
#pragma pack(pop)

    void Execute(const vm_byte* _instr, Thread* _thread);

    constexpr vm_ui64 GetSize(OpCode _opCode)
    {
        switch (_opCode)
        {
        case OpCode::NOOP: return NOOP::GetSize();
        case OpCode::POP: return POP::GetSize();
        case OpCode::RET: return RET::GetSize();
        case OpCode::RETV: return RETV::GetSize();
        case OpCode::ADD: return ADD::GetSize();
        case OpCode::SUB: return SUB::GetSize();
        case OpCode::MUL: return MUL::GetSize();
        case OpCode::DIV: return DIV::GetSize();
        case OpCode::EQ: return EQ::GetSize();
        case OpCode::NEQ: return NEQ::GetSize();
        case OpCode::PUSH: return PUSH::GetSize();
        case OpCode::JUMP: return JUMP::GetSize();
        case OpCode::JUMPNZ: return JUMPNZ::GetSize();
        case OpCode::JUMPZ: return JUMPZ::GetSize();
        case OpCode::CALL: return CALL::GetSize();
        case OpCode::SYSCALL: return SYSCALL::GetSize();
        case OpCode::SLOAD: return SLOAD::GetSize();
        case OpCode::SSTORE: return SSTORE::GetSize();
        case OpCode::MLOAD: return MLOAD::GetSize();
        case OpCode::MSTORE: return MSTORE::GetSize();
        case OpCode::LLOAD: return LLOAD::GetSize();
        case OpCode::LSTORE: return LSTORE::GetSize();
        case OpCode::PLOAD: return PLOAD::GetSize();
        case OpCode::PSTORE: return PSTORE::GetSize();
        case OpCode::CONVERT: return CONVERT::GetSize();
        default: assert(false && "Case not handled");
        }

        return 0;
    }

    std::string ToString(DataType _dt);
    std::string ToString(const vm_byte* _instr);
    void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent);
}