#include "evm.h"
#include <iostream>

#define PRINT_INSTRUCTIONS_ON_EXECUTION false
#define DO_IF(stmt, cond) if constexpr(cond) stmt

namespace Instructions
{
    const size_t Sizes[(size_t)OpCode::_COUNT] = {INSTRUCTION_SIZES};
    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];

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

    VMResult JUMP(Thread *_thread)
    {
        Word word = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        DO_IF(std::cout << "JUMP " << word.as_ptr << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        _thread->instrPtr = word.as_uint - Sizes[(size_t)OpCode::JUMP];
        return VMResult::SUCCESS;
    }

    VMResult EXIT(Thread *_thread)
    {
        DO_IF(std::cout << "EXIT" << std::endl, PRINT_INSTRUCTIONS_ON_EXECUTION);

        Word code;
        VM_PERFORM(_thread->PopStack(code));

        _thread->GetVM()->Quit(VMResult::SUCCESS, code.as_int);
        return VMResult::SUCCESS;
    }

    void Init()
    {
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
        ExecutionFuncs[(size_t)OpCode::EXIT] = &EXIT;
        ExecutionFuncs[(size_t)OpCode::JUMP] = &JUMP;
    }
}