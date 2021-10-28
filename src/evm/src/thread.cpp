#include "thread.h"
#include <mutex>
#include <iostream>
#include "program.h"
#include "vm.h"
#include "instructions.h"
#include "../build.h"

Thread::Thread(VM *_vm, ThreadID _id, vm_ui64 _stackSize, vm_byte *_startIP)
    : vm(_vm), instrPtr(_startIP), id(_id), stackPtr(0ull), framePtr(0ull), isAlive(false)
{
    assert(_stackSize % WORD_SIZE == 0);
    stack = Memory(_stackSize);
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
#ifdef BUILD_DEBUG
                             if (PRINT_STACK_AFTER_THREAD_END)
                                 PrintStack();
#endif
                             isAlive = false;
                         });
    thread.detach();
}

void Thread::Run()
{
    while (vm->IsRunning())
    {
        std::scoped_lock<std::mutex> lock(vm->mutex); //Lock execution to a thread so that this finishes an instruction uninterrupted

        vm_byte opcode = *instrPtr;
        if (opcode >= (size_t)Instructions::OpCode::_COUNT)
            throw VMError::UNKNOWN_OP_CODE(opcode);

#ifdef BUILD_DEBUG
        if (PRINT_INSTR_BEFORE_EXECUTION)
            std::cout << Instructions::ToString(instrPtr) << "\t(Thread ID: " << id << ")" << std::endl;
#endif

        Instructions::ExecutionFuncs[opcode](this);
        instrPtr += Instructions::GetSize((Instructions::OpCode)opcode);

#ifdef BUILD_DEBUG
        if (PRINT_STACK_AFTER_INSTR_EXECUTION)
            PrintStack();
#endif
    }
}

void Thread::Join()
{
    while (isAlive)
        ;
}

void Thread::PrintStack()
{
    std::cout << std::string(40, '=') << "Thread ID: " << id << std::string(40, '=') << std::endl;

    for (uint64_t i = 0; i < stackPtr; i += WORD_SIZE)
    {
        std::cout << ":\ti32: " << ((Word *)&stack[i])->as_i32;
        std::cout << "\ti64: " << ((Word *)&stack[i])->as_i64;
        std::cout << "\tf32: " << ((Word *)&stack[i])->as_f32;
        std::cout << "\tf64: " << ((Word *)&stack[i])->as_f64;
        std::cout << "\tptr: " << ((Word *)&stack[i])->as_ptr;

        if (i + WORD_SIZE == stackPtr)
            std::cout << "\t\t<-------";

        std::cout << std::endl;
    }

    std::cout << std::string(90, '=') << std::endl;
}

void Thread::OffsetSP(vm_i64 _off)
{
    stackPtr += _off;
    if (stackPtr > stack.size())
        throw VMError::STACK_OVERFLOW();
}

void Thread::PushFrame()
{
    PushStack(framePtr);
    framePtr = stackPtr;
}

void Thread::PopFrame()
{
    stackPtr = framePtr;
    if (stackPtr > stack.size())
        throw VMError::STACK_OVERFLOW();
    
    framePtr = PopStack<vm_ui64>();
}
