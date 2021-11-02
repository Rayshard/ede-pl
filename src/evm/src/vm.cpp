#include "vm.h"
#include "thread.h"
#include "instructions.h"
#include <iostream>

VM::VM(bool _runGC)
    : heap(this), runGC(_runGC), threads(), running(false), nextThreadID(0), exitCode(0), stdInput(std::wcin.rdbuf()), stdOutput(std::wcout.rdbuf()) {}

VM::~VM()
{
}

vm_i64 VM::Run(vm_ui64 _stackSize, vm_byte *_startIP, const std::vector<std::string> &_cmdLineArgs)
{
    running = true;

    //Start main thread
    std::vector<Word> args(_cmdLineArgs.size() + 1);

    for (auto it = _cmdLineArgs.rbegin(); it != _cmdLineArgs.rend(); it++)
    {
        auto arg = *it;
        Word argSize = (vm_ui64)arg.size();
        auto ptr = heap.Alloc(arg.size() + WORD_SIZE);

        std::copy((vm_byte*)&argSize, (vm_byte*)&argSize + WORD_SIZE, ptr);
        std::copy((vm_byte*)&arg.front(), (vm_byte*)&arg.back(), ptr + WORD_SIZE);
        args.push_back(ptr);
    }

    args.push_back((vm_ui64)_cmdLineArgs.size()); //Add the number of arguments

    SpawnThread(_stackSize, _startIP, args);

    //Possibly start garbage collector
    if (runGC)
        heap.StartGC();

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

    //Stop garbage collector if it was started
    if (runGC)
        heap.StopGC();

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

ThreadID VM::SpawnThread(vm_ui64 _stackSize, vm_byte *_startIP, const std::vector<Word> &_args)
{
    if (!running)
        throw VMError::CANNOT_SPAWN_THREAD();

    auto id = nextThreadID++;
    threads.emplace(id, Thread(this, id, _stackSize, _startIP));
    threads.at(id).Start(_args);
    return id;
}

Thread &VM::GetThread(ThreadID _id)
{
    auto idSearch = threads.find(_id);
    if (idSearch == threads.end())
        throw VMError::INVALID_THREAD_ID(_id);

    return idSearch->second;
}

void VM::SetStdIO(std::wstreambuf *_in, std::wstreambuf *_out)
{
    stdInput.rdbuf(_in ? _in : std::wcin.rdbuf());
    stdOutput.rdbuf(_out ? _out : std::wcout.rdbuf());
}