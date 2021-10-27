#include "vm.h"
#include "thread.h"
#include "instructions.h"
#include <iostream>

VM::VM()
    : heap(), threads(), running(false), nextThreadID(0), exitCode(0), stdInput(std::wcin.rdbuf()), stdOutput(std::wcout.rdbuf()) {}

VM::~VM()
{
    
}

int64_t VM::Run(size_t _stackSize, byte *_startIP)
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

size_t VM::SpawnThread(size_t _stackSize, byte *_startIP)
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

void VM::SetStdIO(std::wstreambuf *_in, std::wstreambuf *_out)
{
    stdInput.rdbuf(_in ? _in : std::wcin.rdbuf());
    stdOutput.rdbuf(_out ? _out : std::wcout.rdbuf());
}

byte *VM::Malloc(size_t _amt)
{
    std::vector<byte> buffer(_amt);
    byte* address = buffer.data();

    heap.emplace(address, std::move(buffer));
    return address;
}

void VM::Free(byte *_addr)
{
    auto addrSearch = heap.find(_addr);
    if (addrSearch == heap.end())
        throw VMError::MEMORY_NOT_ALLOCATED();

    heap.erase(_addr);
}

bool VM::IsAllocated(byte *_addr)
{
    return heap.find(_addr) != heap.end();
}