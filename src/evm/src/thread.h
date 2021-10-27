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
    uint64_t stackPtr;

    std::thread thread;
    bool isAlive;

public:
    byte *instrPtr;
    uint64_t framePtr;

    Thread(VM *_vm, size_t _id, uint64_t _stackSize, byte *_startIP);

    void Start();
    void Join();
    void Run();

    void OffsetSP(int64_t _off);
    void SetSP(int64_t _pos);
    void PrintStack();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
    bool IsAlive() { return isAlive; }
    uint64_t GetSP() { return stackPtr; }
    uint64_t GetFP() { return framePtr; }

    template <typename T>
    T ReadStack(int64_t _pos)
    {
        if (_pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (_pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        return *(T *)&stack[_pos];
    }

    template <typename T>
    void WriteStack(int64_t _pos, const T &_value)
    {
        if (_pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (_pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        std::copy((byte *)&_value, (byte *)&_value + sizeof(T), &stack[_pos]);
    }

    template <typename T>
    void PushStack(const T &_value)
    {
        WriteStack(stackPtr, _value);
        stackPtr += sizeof(T);
    }

    template <typename T>
    T PopStack()
    {
        auto result = ReadStack<T>(stackPtr - sizeof(T));
        stackPtr -= sizeof(T);
        return result;
    }
};
