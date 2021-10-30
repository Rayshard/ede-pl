#include "vm.h"
#include "thread.h"
#include "instructions.h"
#include <iostream>

VM::VM()
    : heap(), threads(), running(false), nextThreadID(0), exitCode(0), stdInput(std::wcin.rdbuf()), stdOutput(std::wcout.rdbuf()) {}

VM::~VM()
{
}

vm_i64 VM::Run(vm_ui64 _stackSize, vm_byte *_startIP)
{
    running = true;
    SpawnThread(_stackSize, _startIP);

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
                thread.Join();
                deadThreads.push_back(id);
            }
        }

        for (auto id : deadThreads)
            threads.erase(id);

        mutex.unlock();
    }

    running = false;

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

ThreadID VM::SpawnThread(vm_ui64 _stackSize, vm_byte *_startIP)
{
    if (!running)
        throw VMError::CANNOT_SPAWN_THREAD();

    auto id = nextThreadID++;
    threads.emplace(id, Thread(this, id, _stackSize, _startIP));
    threads.at(id).Start();
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