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

    VMResult NOOP(Thread *_thread)
    {
        return VMResult::SUCCESS;
    }

    VMResult PUSH(Thread *_thread)
    {
        Word word = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "PUSH " << word.as_int << "|" << word.as_double << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        return _thread->PushStack(word);
    }

    VMResult POP(Thread *_thread)
    {
        DO_IF(std::cout << "POP" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word trash;
        return _thread->PopStack(trash);
    }

    VMResult IADD(Thread *_thread)
    {
        DO_IF(std::cout << "IADD" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int + right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult ISUB(Thread *_thread)
    {
        DO_IF(std::cout << "ISUB" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int - right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult IMUL(Thread *_thread)
    {
        DO_IF(std::cout << "IMUL" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int * right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult IDIV(Thread *_thread)
    {
        DO_IF(std::cout << "IDIV" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        if (right.as_int == 0)
            return VMResult::DIV_BY_ZERO;

        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int / right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult DADD(Thread *_thread)
    {
        DO_IF(std::cout << "DADD" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double + right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DSUB(Thread *_thread)
    {
        DO_IF(std::cout << "DSUB" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double - right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DMUL(Thread *_thread)
    {
        DO_IF(std::cout << "DMUL" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double * right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DDIV(Thread *_thread)
    {
        DO_IF(std::cout << "DDIV" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        if (right.as_double == 0)
            return VMResult::DIV_BY_ZERO;

        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double / right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult EQ(Thread *_thread)
    {
        DO_IF(std::cout << "EQ" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, result;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        result.as_uint = left.as_uint == right.as_uint;
        return _thread->PushStack(result);
    }

    VMResult NEQ(Thread *_thread)
    {
        DO_IF(std::cout << "NEQ" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word left, right, result;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        result.as_uint = left.as_uint != right.as_uint ? 1u : 0u;
        return _thread->PushStack(result);
    }

    VMResult JUMP(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMP " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMP);
        return VMResult::SUCCESS;
    }

    VMResult JUMPNZ(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMPNZ " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word condition;
        VM_PERFORM(_thread->PopStack(condition));

        if (condition.as_uint != 0u)
            _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMPNZ);

        return VMResult::SUCCESS;
    }

    VMResult JUMPZ(Thread *_thread)
    {
        Word target = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMPZ " << target.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word condition;
        VM_PERFORM(_thread->PopStack(condition));

        if (condition.as_uint == 0u)
            _thread->instrPtr = target.as_uint - GetSize(OpCode::JUMPZ);

        return VMResult::SUCCESS;
    }

    VMResult SYSCALL_EXIT(Thread *_thread)
    {
        DO_IF(std::cout << "SYSCALL EXIT" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word code;
        VM_PERFORM(_thread->PopStack(code));

        _thread->GetVM()->Quit(VMResult::SUCCESS, code.as_int);
        return VMResult::SUCCESS;
    }

    VMResult SYSCALL(Thread *_thread)
    {
        byte code = _thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        if (code >= (size_t)SysCallCode::_COUNT)
            return VMResult::UNKNOWN_SYSCALL_CODE;

        return SysCallExecutionFuncs[code](_thread);
    }

#pragma region Loads and Stores
    VMResult SLOAD(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "SLOAD " << offset << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word result;
        VM_PERFORM(_thread->ReadStack(offset, result));

        return _thread->PushStack(result);
    }

    VMResult SSTORE(Thread *_thread)
    {
        int64_t offset = *(int64_t *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "SSTORE " << offset << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word value;
        VM_PERFORM(_thread->PopStack(value));

        return _thread->WriteStack(offset, value);
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
    }
}