#include "instructions.h"
#include <iostream>
#include "thread.h"
#include "vm.h"

#define PRINT_INSTRUCTIONS_ON_EXECUTION false
#define DO_IF(stmt, cond) \
    if constexpr (cond)   \
    stmt

namespace Instructions
{
    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];
    ExecutionFunc SysCallExecutionFuncs[(size_t)SysCallCode::_COUNT];

    void NOOP(Thread *_thread)
    {
        return;
    }

    void PUSH(Thread *_thread)
    {
        Word word = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "PUSH " << word.as_int << "|" << word.as_double << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        _thread->PushStack(word);
    }

    void POP(Thread *_thread)
    {
        DO_IF(std::cout << "POP" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PopStack<Word>();
    }

#pragma region Binops
    void IADD(Thread *_thread)
    {
        DO_IF(std::cout << "IADD" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(_thread->PopStack<int64_t>() + _thread->PopStack<int64_t>());
    }

    void ISUB(Thread *_thread)
    {
        DO_IF(std::cout << "ISUB" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
_thread->PushStack(_thread->PopStack<int64_t>() - _thread->PopStack<int64_t>());
    }

    void IMUL(Thread *_thread)
    {
        DO_IF(std::cout << "IMUL" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(_thread->PopStack<int64_t>() * _thread->PopStack<int64_t>());
    }

    void IDIV(Thread *_thread)
    {
        DO_IF(std::cout << "IDIV" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        int64_t left = _thread->PopStack<int64_t>(), right = _thread->PopStack<int64_t>();
        if (right == 0ll)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void DADD(Thread *_thread)
    {
        DO_IF(std::cout << "DADD" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(_thread->PopStack<double>() + _thread->PopStack<double>());
    }

    void DSUB(Thread *_thread)
    {
        DO_IF(std::cout << "DSUB" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
_thread->PushStack(_thread->PopStack<double>() - _thread->PopStack<double>());
    }

    void DMUL(Thread *_thread)
    {
        DO_IF(std::cout << "DMUL" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(_thread->PopStack<double>() * _thread->PopStack<double>());
    }

    void DDIV(Thread *_thread)
    {
        DO_IF(std::cout << "DDIV" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        double left = _thread->PopStack<double>(), right = _thread->PopStack<double>();
        if (right == 0.0)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void EQ(Thread *_thread)
    {
        DO_IF(std::cout << "EQ" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(uint64_t(_thread->PopStack<uint64_t>() == _thread->PopStack<uint64_t>()));
    }

    void NEQ(Thread *_thread)
    {
        DO_IF(std::cout << "NEQ" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->PushStack(uint64_t(_thread->PopStack<uint64_t>() != _thread->PopStack<uint64_t>()));
    }

    void JUMP(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMP " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMP);
    }

    void JUMPNZ(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMPNZ " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        if (_thread->PopStack<uint64_t>() != 0ull)
            _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMPNZ);
    }

    void JUMPZ(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMPZ " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        if (_thread->PopStack<uint64_t>() == 0ull)
            _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMPZ);
    }
#pragma endregion

#pragma region Syscalls
    void SYSCALL_EXIT(Thread *_thread)
    {
        DO_IF(std::cout << "SYSCALL EXIT" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        _thread->GetVM()->Quit(_thread->PopStack<int64_t>());
    }

    void SYSCALL_PRINTC(Thread *_thread)
    {
        DO_IF(std::cout << "SYSCALL PRINTC" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);
        std::wcout << static_cast<wchar_t>(_thread->PopStack<int64_t>());
    }

    void SYSCALL(Thread *_thread)
    {
        byte code = _thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        if (code >= (size_t)SysCallCode::_COUNT)
            throw VMError::UNKNOWN_SYSCALL_CODE();

        SysCallExecutionFuncs[code](_thread);
    }
#pragma endregion


#pragma region Loads and Stores
    void SLOAD(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "SLOAD " << offset << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        _thread->PushStack(_thread->ReadStack<Word>(offset));
    }

    void SSTORE(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "SSTORE " << offset << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

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

        SysCallExecutionFuncs[(size_t)SysCallCode::EXIT] = &SYSCALL_EXIT;
        SysCallExecutionFuncs[(size_t)SysCallCode::PRINTC] = &SYSCALL_PRINTC;
    }
}