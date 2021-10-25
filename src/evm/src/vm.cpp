#include "evm.h"

VM::VM(Program &&_program)
    : program(std::move(_program)), heap(), threads(), running(false), nextThreadID(0)
    {
        exitInfo.code = 0;
        exitInfo.result = VMResult::SUCCESS;
    }

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
