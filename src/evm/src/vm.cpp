#include "vm.h"
#include "thread.h"

VM::VM(Program &&_program)
    : program(std::move(_program)), heap(), threads(), running(false), nextThreadID(0), exitCode(0) {}

VM::~VM()
{
    for (auto &[ptr, _] : heap)
        delete[] ptr;

    heap.clear();
}

int64_t VM::Run(size_t _stackSize)
{
    running = true;
    SpawnThread(_stackSize, 0);

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

size_t VM::SpawnThread(size_t _stackSize, size_t _startIP)
{
    if (!running)
        throw VMError::CANNOT_SPAWN_THREAD();

    auto id = nextThreadID++;
    threads.emplace(id, Thread(this, id, _stackSize, _startIP));
    threads.at(id).Start();
    return id;
}

void VM::JoinThread(size_t _id)
{
    auto idSearch = threads.find(_id);
    if (idSearch == threads.end())
        return;

    idSearch->second.Join();
}
