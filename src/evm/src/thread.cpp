#include "thread.h"
#include <mutex>
#include <iostream>
#include "program.h"
#include "vm.h"
#include "instructions.h"

#define PRINT_INSTRUCTIONS_ON_EXECUTION false
#define PRINT_STACK_AFTER_INSTR_EXECUTION false

Thread::Thread(VM *_vm, size_t _id, uint64_t _stackSize, byte* _startIP)
    : vm(_vm), instrPtr(_startIP), id(_id), stackPtr(0), framePtr(0), isAlive(false)
{
    assert(_stackSize % WORD_SIZE == 0);
    stack = std::vector<byte>(_stackSize);
}

void Thread::Start()
{
    isAlive = true;
    thread = std::thread([this]
                         {
                             try
                             {
                                 this->Run();
                             }
                             catch (const VMError &e)
                             {
                                 std::scoped_lock<std::mutex> lock(vm->mutex);
                                 this->vm->Quit(e);
                             }

                             std::scoped_lock<std::mutex> lock(vm->mutex);
                             isAlive = false;
                         });
    thread.detach();
}

void Thread::Run()
{
    while (vm->IsRunning())
    {
        std::scoped_lock<std::mutex> lock(vm->mutex); //Lock execution to a thread so that this finishes an instruction uninterrupted

        byte opcode = *instrPtr;
        if (opcode >= (size_t)Instructions::OpCode::_COUNT)
            throw VMError::UNKNOWN_OP_CODE();
        
        #if PRINT_INSTRUCTIONS_ON_EXECUTION
            std::cout << Instructions::ToString(instrPtr) << std::endl;
        #endif

        Instructions::ExecutionFuncs[opcode](this);
        instrPtr += Instructions::GetSize((Instructions::OpCode)opcode);
        
        #if PRINT_STACK_AFTER_INSTR_EXECUTION
           PrintStack();
        #endif
    }
}

void Thread::Join()
{
    while (isAlive)
        ;
}

void Thread::SetSP(int64_t _pos)
{
    stackPtr = _pos;
    if (stackPtr < 0)
        throw VMError::STACK_UNDERFLOW();
    else if (stackPtr > stack.size())
        throw VMError::STACK_OVERFLOW();
}

void Thread::PrintStack()
{
    std::cout << std::string(40, '=') << "Thread ID: " << id << std::string(40, '=') << std::endl;

    for (uint64_t i = 0; i < stackPtr; i += WORD_SIZE)
    {
        std::cout << reinterpret_cast<void *>(&stack[i]);
        std::cout << ":\ti64: " << ((Word *)&stack[i])->as_int;
        std::cout << "\tf64: " << ((Word *)&stack[i])->as_double;

        if (i + WORD_SIZE == stackPtr)
            std::cout << "\t\t<-------";

        std::cout << std::endl;
    }

    std::cout << std::string(90, '=') << std::endl;
}
