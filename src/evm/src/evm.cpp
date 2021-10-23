#include "evm.h"
#include <iostream>
#include <fstream>

namespace Instructions
{
    typedef VMResult (*ExecutionFunc)(Thread *);
    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];

    VMResult PUSH(Thread *_thread)
    {
        Word word = *(Word *)&_thread->GetVM()->GetProgram()[_thread->instrPtr + OP_CODE_SIZE];
        std::cout << "PUSH " << word.as_int << "|" << word.as_double << std::endl;

        return _thread->PushStack(word);
    }

    VMResult POP(Thread *_thread)
    {
        std::cout << "POP" << std::endl;

        Word trash;
        return _thread->PopStack(trash);
    }

    VMResult IADD(Thread *_thread)
    {
        std::cout << "IADD" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int + right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult ISUB(Thread *_thread)
    {
        std::cout << "ISUB" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int - right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult IMUL(Thread *_thread)
    {
        std::cout << "IMUL" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_int = left.as_int * right.as_int;
        return _thread->PushStack(sum);
    }

    VMResult IDIV(Thread *_thread)
    {
        std::cout << "IDIV" << std::endl;

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
        std::cout << "DADD" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double + right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DSUB(Thread *_thread)
    {
        std::cout << "DSUB" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double - right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DMUL(Thread *_thread)
    {
        std::cout << "DMUL" << std::endl;

        Word left, right, sum;
        VM_PERFORM(_thread->PopStack(right));
        VM_PERFORM(_thread->PopStack(left));

        sum.as_double = left.as_double * right.as_double;
        return _thread->PushStack(sum);
    }

    VMResult DDIV(Thread *_thread)
    {
        std::cout << "DDIV" << std::endl;

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
        std::cout << "JUMP " << word.as_ptr << std::endl;

        _thread->instrPtr = word.as_uint - InstructionSizes[(size_t)OpCode::JUMP];
        return VMResult::SUCCESS;
    }

    VMResult EXIT(Thread *_thread)
    {
        std::cout << "EXIT" << std::endl;

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

Thread::Thread(VM *_vm, size_t _id, size_t _stackSize, size_t _startIP)
    : vm(_vm), instrPtr(_startIP), id(_id), stackPtr(0), execResult()
{
    assert(_stackSize % WORD_SIZE == 0);
    stack = std::vector<byte>(_stackSize);
}

void Thread::Start()
{
    thread = std::thread([this]
                         {
                             this->execResult = this->Run();

                             if (this->execResult.value() != VMResult::SUCCESS)
                                 this->vm->Quit(this->execResult.value(), 0);

                             vm->mutex.unlock(); //Unlock execution just in case Run returned before unlocking
                         });
}

VMResult Thread::Run()
{
    const Program &program = vm->GetProgram();

    while (vm->IsRunning())
    {
        vm->mutex.lock(); //Lock execution to a thread so that they finish an instruction uninterrupted

        if (instrPtr < 0 || instrPtr >= program.size())
            return VMResult::IP_OUT_OF_BOUNDS;

        byte opcode = program[instrPtr];
        if (opcode >= (size_t)OpCode::_COUNT)
            return VMResult::UNKNOWN_OP_CODE;
        else if (instrPtr + InstructionSizes[opcode] > program.size())
            return VMResult::IP_OVERFLOW;

        VM_PERFORM(Instructions::ExecutionFuncs[opcode](this));
        instrPtr += InstructionSizes[opcode];

        PrintStack();

        vm->mutex.unlock(); //Unlock execution
    }

    return VMResult::SUCCESS;
}

void Thread::Join()
{
    thread.join();
}

VMResult Thread::PushStack(Word _word)
{
    if (stackPtr + WORD_SIZE > stack.size())
        return VMResult::STACK_OVERFLOW;

    std::copy(_word.bytes, _word.bytes + WORD_SIZE, &stack[stackPtr]);
    stackPtr += WORD_SIZE;
    return VMResult::SUCCESS;
}

VMResult Thread::PopStack(Word &word)
{
    if (stackPtr < WORD_SIZE)
        return VMResult::STACK_UNDERFLOW;

    word = *(Word *)&stack[stackPtr - WORD_SIZE];
    stackPtr -= WORD_SIZE;
    return VMResult::SUCCESS;
}

void Thread::PrintStack()
{
    std::cout << std::string(40, '=') << "Thread ID: " << id << std::string(40, '=') << std::endl;

    for (size_t i = 0; i < stack.size(); i += WORD_SIZE)
    {
        std::cout << reinterpret_cast<void *>(&stack[i]);
        std::cout << ":\t" << ((Word *)&stack[i])->as_int;
        std::cout << "\t" << ((Word *)&stack[i])->as_double;

        if (i + WORD_SIZE == stackPtr)
            std::cout << "\t\t<-------";

        std::cout << std::endl;
    }

    std::cout << std::string(90, '=') << std::endl;
}

bool Thread::IsAlive() { return !execResult.has_value(); }

VM::VM(Program &&_program)
    : program(std::move(_program)), heap(), threads(), running(false), nextThreadID(0) {}

VM::~VM()
{
    for (auto &[ptr, _] : heap)
        delete[] ptr;

    heap.clear();
}

VMResult VM::Run(size_t _stackSize)
{
    running = true;

    size_t initialThreadID;
    VM_PERFORM(SpawnThread(_stackSize, 0, initialThreadID));

    while (threads.size() != 0)
    {
        if (!running)
        {
            for (auto &[_, thread] : threads)
                thread.Join();

            threads.clear();
            break;
        }

        mutex.lock();

        std::vector<size_t> deadThreads;
        for (auto &[id, thread] : threads)
        {
            if (!thread.IsAlive())
            {
                thread.Join();
                deadThreads.push_back(id);
            }
        }

        for (auto id : deadThreads)
            threads.erase(id);

        mutex.unlock();
    }

    running = false;
    return exitInfo.result;
}

void VM::Quit(VMResult _res, int64_t _code)
{
    if (!running)
        return;

    exitInfo.code = _code;
    exitInfo.result = _res;
    running = false;
}

VMResult VM::SpawnThread(size_t _stackSize, size_t _startIP, size_t &_id)
{
    if (!running)
        return VMResult::CANNOT_SPAWN_THREAD;

    _id = nextThreadID++;
    threads.emplace(_id, Thread(this, _id, _stackSize, _startIP));
    threads.at(_id).Start();

    return VMResult::SUCCESS;
}

void VM::JoinThread(size_t _id)
{
    auto idSearch = threads.find(_id);
    if (idSearch == threads.end())
        return;

    idSearch->second.Join();
}

template <class type>
void Insert(Program &_prog, type _value) { _prog.insert(_prog.end(), (char *)&_value, (char *)&_value + sizeof(_value)); }

int main(int argc, char *argv[])
{
    assert(argc == 2 && "Expected path to Ede program file");
    Instructions::Init();

    //Open file for reading
    std::ifstream file(argv[1], std::ios::ate); //The second arg tells the stream to open and seek to the end of the file
    if (!file.is_open())
    {
        std::cout << "Unable to open program file!" << std::endl;
        return -1;
    }

    //Read the program file
    // Program program(file.tellg());
    // file.seekg(0, std::ios::beg);

    // if (!file.read((char *)program.data(), program.size()))
    // {
    //     std::cout << "Unable to read program file!" << std::endl;
    //     return -1;
    // }

    Program program = Program();

    Insert(program, OpCode::PUSH);
    Insert(program, (int64_t)123);
    Insert(program, OpCode::PUSH);
    Insert(program, (int64_t)456);
    Insert(program, OpCode::IADD);
    Insert(program, OpCode::PUSH);
    Insert(program, (int64_t)15);
    Insert(program, OpCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    if (result == VMResult::SUCCESS)
    {
        std::cout << "Exited with code " << vm.GetExitCode() << "." << std::endl;
        return vm.GetExitCode();
    }

    std::cout << VMResultStrings[(size_t)result] << std::endl;
    return -1;
}