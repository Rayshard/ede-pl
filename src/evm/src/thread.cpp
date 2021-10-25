#include "thread.h"
#include <mutex>
#include <iostream>
#include "program.h"
#include "vm.h"
#include "instructions.h"

Thread::Thread(VM *_vm, size_t _id, size_t _stackSize, size_t _startIP)
    : vm(_vm), instrPtr(_startIP), id(_id), stackPtr(0), isAlive(false)
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
    const Program &program = vm->GetProgram();

    while (vm->IsRunning())
    {
        std::scoped_lock<std::mutex> lock(vm->mutex); //Lock execution to a thread so that this finishes an instruction uninterrupted

        if (instrPtr < 0 || instrPtr >= program.size())
            throw VMError::IP_OUT_OF_BOUNDS();

        byte opcode = program[instrPtr];
        if (opcode >= (size_t)Instructions::OpCode::_COUNT)
            throw VMError::UNKNOWN_OP_CODE();
        else if (instrPtr + Instructions::GetSize((Instructions::OpCode)opcode) > program.size())
            throw VMError::IP_OVERFLOW();

        Instructions::ExecutionFuncs[opcode](this);
        //PrintStack();
        instrPtr += Instructions::GetSize((Instructions::OpCode)opcode);
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

    for (size_t i = 0; i < stackPtr; i += WORD_SIZE)
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
