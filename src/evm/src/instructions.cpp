#include "instructions.h"
#include <iostream>
#include "thread.h"
#include "vm.h"

namespace Instructions
{
    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];
    ExecutionFunc SysCallExecutionFuncs[(size_t)SysCallCode::_COUNT];

    void NOOP(Thread *_thread)
    {
        return;
    }

#pragma region Binops
    void IADD(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<int64_t>() + _thread->PopStack<int64_t>());
    }

    void ISUB(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<int64_t>() - _thread->PopStack<int64_t>());
    }

    void IMUL(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<int64_t>() * _thread->PopStack<int64_t>());
    }

    void IDIV(Thread *_thread)
    {
        int64_t left = _thread->PopStack<int64_t>(), right = _thread->PopStack<int64_t>();
        if (right == 0ll)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void DADD(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<double>() + _thread->PopStack<double>());
    }

    void DSUB(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<double>() - _thread->PopStack<double>());
    }

    void DMUL(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<double>() * _thread->PopStack<double>());
    }

    void DDIV(Thread *_thread)
    {

        double left = _thread->PopStack<double>(), right = _thread->PopStack<double>();
        if (right == 0.0)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void EQ(Thread *_thread)
    {
        _thread->PushStack(uint64_t(_thread->PopStack<uint64_t>() == _thread->PopStack<uint64_t>()));
    }

    void NEQ(Thread *_thread)
    {
        _thread->PushStack(uint64_t(_thread->PopStack<uint64_t>() != _thread->PopStack<uint64_t>()));
    }

#pragma endregion

#pragma region Branching
    void JUMP(Thread *_thread)
    {
        uint64_t target = *(uint64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->instrPtr = target - GetSize(OpCode::JUMP);
    }

    void JUMPNZ(Thread *_thread)
    {
        if (_thread->PopStack<uint64_t>() == 0ull)
            return;

        uint64_t target = *(uint64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->instrPtr = target - GetSize(OpCode::JUMPNZ);
    }

    void JUMPZ(Thread *_thread)
    {
        if (_thread->PopStack<uint64_t>() != 0ull)
            return;

        uint64_t target = *(uint64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->instrPtr = target - GetSize(OpCode::JUMPZ);
    }

#pragma endregion

#pragma region Syscalls
    void SYSCALL_EXIT(Thread *_thread)
    {
        _thread->GetVM()->Quit(_thread->PopStack<int64_t>());
    }

    void SYSCALL_PRINTC(Thread *_thread)
    {
        _thread->GetVM()->GetStdOut() << static_cast<wchar_t>(_thread->PopStack<int64_t>());
    }

    void SYSCALL(Thread *_thread)
    {
        byte code = _thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        if (code >= (size_t)SysCallCode::_COUNT)
            throw VMError::UNKNOWN_SYSCALL_CODE();

        SysCallExecutionFuncs[code](_thread);
    }
#pragma endregion

#pragma region Converters
    void I2D(Thread *_thread)
    {
        _thread->PushStack((double)_thread->PopStack<int64_t>());
    }

    void D2I(Thread *_thread)
    {
        _thread->PushStack((int64_t)_thread->PopStack<double>());
    }
#pragma endregion

#pragma region Loads and Stores
    void PUSH(Thread *_thread)
    {
        Word word = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->PushStack(word);
    }

    void POP(Thread *_thread)
    {
        _thread->PopStack<Word>();
    }

    void SLOAD(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(offset));
    }

    void SSTORE(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        _thread->WriteStack(offset, _thread->PopStack<Word>());
    }
#pragma endregion

    void Init()
    {
        ExecutionFuncs[(size_t)OpCode::NOOP] = &NOOP;
        ExecutionFuncs[(size_t)OpCode::PUSH] = &PUSH;
        ExecutionFuncs[(size_t)OpCode::POP] = &POP;
        ExecutionFuncs[(size_t)OpCode::IADD] = &IADD;
        ExecutionFuncs[(size_t)OpCode::ISUB] = &ISUB;
        ExecutionFuncs[(size_t)OpCode::IMUL] = &IMUL;
        ExecutionFuncs[(size_t)OpCode::IDIV] = &IDIV;
        ExecutionFuncs[(size_t)OpCode::DADD] = &DADD;
        ExecutionFuncs[(size_t)OpCode::DSUB] = &DSUB;
        ExecutionFuncs[(size_t)OpCode::DMUL] = &DMUL;
        ExecutionFuncs[(size_t)OpCode::DDIV] = &DDIV;
        ExecutionFuncs[(size_t)OpCode::JUMP] = &JUMP;
        ExecutionFuncs[(size_t)OpCode::JUMPZ] = &JUMPZ;
        ExecutionFuncs[(size_t)OpCode::JUMPNZ] = &JUMPNZ;
        ExecutionFuncs[(size_t)OpCode::SYSCALL] = &SYSCALL;
        ExecutionFuncs[(size_t)OpCode::EQ] = &EQ;
        ExecutionFuncs[(size_t)OpCode::NEQ] = &NEQ;
        ExecutionFuncs[(size_t)OpCode::SLOAD] = &SLOAD;
        ExecutionFuncs[(size_t)OpCode::SSTORE] = &SSTORE;
        ExecutionFuncs[(size_t)OpCode::I2D] = &I2D;
        ExecutionFuncs[(size_t)OpCode::D2I] = &D2I;

        SysCallExecutionFuncs[(size_t)SysCallCode::EXIT] = &SYSCALL_EXIT;
        SysCallExecutionFuncs[(size_t)SysCallCode::PRINTC] = &SYSCALL_PRINTC;
    }

    std::string ToString(const byte *_instr)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::NOOP:
            return "NOOP";
        case OpCode::I2D:
            return "I2D";
        case OpCode::D2I:
            return "D2I";
        case OpCode::POP:
            return "POP";
        case OpCode::IADD:
            return "IADD";
        case OpCode::ISUB:
            return "ISUB";
        case OpCode::IMUL:
            return "IMUL";
        case OpCode::IDIV:
            return "IDIV";
        case OpCode::DADD:
            return "DADD";
        case OpCode::DSUB:
            return "DSUB";
        case OpCode::DMUL:
            return "DMUL";
        case OpCode::DDIV:
            return "DDIV";
        case OpCode::EQ:
            return "EQ";
        case OpCode::NEQ:
            return "NEQ";
        case OpCode::PUSH:
            return "PUSH " + Hex(*(uint64_t *)&_instr[1]);
        case OpCode::JUMP:
            return "JUMP " + Hex(*(uint64_t *)&_instr[1]);
        case OpCode::JUMPNZ:
            return "JUMPNZ " + Hex(*(uint64_t *)&_instr[1]);
        case OpCode::JUMPZ:
            return "JUMPZ " + Hex(*(uint64_t *)&_instr[1]);
        case OpCode::SYSCALL:
        {
            switch ((SysCallCode)_instr[1])
            {
            case SysCallCode::EXIT:
                return "SYSCALL EXIT";
            case SysCallCode::PRINTC:
                return "SYSCALL PRINTC";
            case SysCallCode::MALLOC:
                return "SYSCALL MALLOC";
            case SysCallCode::FREE:
                return "SYSCALL FREE";
            default:
                assert(false && "Case not handled");
            }
        } break;
        case OpCode::SLOAD:
            return "SLOAD " + std::to_string(*(int64_t *)&_instr[1]);
        case OpCode::SSTORE:
            return "SSTORE " + std::to_string(*(int64_t *)&_instr[1]);
        default:
            assert(false && "Case not handled");
        }

        return "";
    }
}