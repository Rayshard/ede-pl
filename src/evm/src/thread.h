#pragma once
#include "evm.h"
#include <thread>
#include <vector>
#include <optional>
#include "vm.h"

class Thread
{
private:
    VM *vm;
    size_t id;
    std::vector<byte> stack;
    size_t stackPtr;

    std::thread thread;
    bool isAlive;

public:
    size_t instrPtr;

    Thread(VM *_vm, size_t _id, size_t _stackSize, size_t _startIP);

    void Start();
    void Join();
    void Run();

    void PrintStack();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
    bool IsAlive() { return isAlive; }

    ///Reads the stack relative to the stack pointer
    template <typename T>
    T ReadStack(int64_t _offset)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        return *(T *)&stack[pos];
    }

    ///Writes to the stack relative to the stack pointer
    template <typename T>
    void WriteStack(size_t _offset, const T &_value)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        std::copy((byte *)&_value, (byte *)&_value + sizeof(T), &stack[pos]);
    }

    template <typename T>
    void PushStack(const T &_value)
    {
        WriteStack(0, _value);
        stackPtr += sizeof(T);
    }

    template <typename T>
    T PopStack()
    {
        auto result = ReadStack<T>(-8);
        stackPtr -= sizeof(T);
        return result;
    }
};
