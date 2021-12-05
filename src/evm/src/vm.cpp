#include "vm.h"
#include "thread.h"
#include "instructions.h"
#include "../build.h"
#include <iostream>

VM::VM()
    : heap(this), threads(), running(false), nextThreadID(0), exitCode(0), stdInput(std::cin.rdbuf()), stdOutput(std::cout.rdbuf()) {}

VM::~VM()
{
}

vm_i64 VM::Run(vm_ui64 _stackSize, Program& _prog, const std::vector<std::string> &_cmdLineArgs)
{
    running = true;

    //Allocate space for globals
    globalsArrayPtr = _prog.GetHeader().numGlobals == 0 ? nullptr : heap.Alloc(_prog.GetHeader().numGlobals * WORD_SIZE);

    // Store command line arguments
    auto argsArraySize = (vm_ui64)_cmdLineArgs.size();
    auto argsArrayPtr = heap.Alloc(VM_UI64_SIZE + _cmdLineArgs.size() * VM_PTR_SIZE);
    auto argsArrayPtrValues = (vm_byte **)&argsArrayPtr[VM_UI64_SIZE];

    *(vm_ui64*)argsArrayPtr = argsArraySize; //Store number of cmd line args

    //Store each cmd line arg
    for (vm_ui64 iArg = 0; iArg < argsArraySize; iArg++)
    {
        auto arg = _cmdLineArgs[iArg];
        Word argSize = (vm_ui64)arg.size();
        auto argPtr = (argsArrayPtrValues[iArg] = heap.Alloc(arg.size() + VM_UI64_SIZE)); //store string ptr

        *(Word*)argPtr = argSize; //store string size
        std::copy(arg.begin(), arg.end(), argPtr + WORD_SIZE);                   //store string chars
    }

    //Start main thread
    SpawnThread(_stackSize, _prog.GetEntryPtr(), {argsArrayPtr});

    //Manage threads
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

        std::vector<ThreadID> deadThreads;
        for (auto &[id, thread] : threads)
        {
            if (!thread.IsAlive())
            {
                thread.Join(); //Even though the thread is dead, we have to call this so that the cpp thread object is destroyed
                deadThreads.push_back(id);
            }
        }

        for (auto id : deadThreads)
            threads.erase(id);

        mutex.unlock();
    }

    running = false;

#ifdef BUILD_DEBUG
    if (PRINT_HEAP_AFTER_PROGRAM_END)
        heap.Print();
#endif

    //Return exit code or error
    if (const VMError *error = std::get_if<VMError>(&exitCode))
        throw *error;

    return std::get<int64_t>(exitCode);
}

void VM::Quit(VMExitCode _code)
{
    if (!running)
        return;

    exitCode = _code;
    running = false;
}

ThreadID VM::SpawnThread(vm_ui64 _stackSize, const vm_byte *_startIP, const std::vector<Word> &_args)
{
    if (!running)
        throw VMError::CANNOT_SPAWN_THREAD();

    auto id = nextThreadID++;
    threads.emplace(id, Thread(this, id, _stackSize, _startIP));
    threads.at(id).Start(globalsArrayPtr, _args);
    return id;
}

Thread &VM::GetThread(ThreadID _id)
{
    auto idSearch = threads.find(_id);
    if (idSearch == threads.end())
        throw VMError::INVALID_THREAD_ID(_id);

    return idSearch->second;
}

void VM::SetStdIO(std::streambuf *_in, std::streambuf *_out)
{
    stdInput.rdbuf(_in ? _in : std::cin.rdbuf());
    stdOutput.rdbuf(_out ? _out : std::cout.rdbuf());
}

DebuggerInfo::DebuggerInfo() : enabled(false), rID(), wID() { }