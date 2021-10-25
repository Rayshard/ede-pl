#pragma once
#include "evm.h"
#include <thread>
#include <vector>
#include <optional>

class Thread
{
private:
    VM *vm;
    size_t id;
    std::vector<byte> stack;
    size_t stackPtr;

    std::thread thread;
    bool isAlive;
    std::optional<VMResult> execResult;

public:
    size_t instrPtr;

    Thread(VM *_vm, size_t _id, size_t _stackSize, size_t _startIP);

    void Start();
    void Join();
    VMResult Run();

    void PrintStack();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
    bool IsAlive() { return isAlive; }

    ///Reads the stack relative to the stack pointer
    template <typename T>
    VMResult ReadStack(int64_t _offset, T &_value)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            return VMResult::STACK_UNDERFLOW;
        else if (pos + sizeof(T) > stack.size())
            return VMResult::STACK_OVERFLOW;

        _value = *(T *)&stack[pos];
        return VMResult::SUCCESS;
    }

    ///Writes to the stack relative to the stack pointer
    template <typename T>
    VMResult WriteStack(size_t _offset, const T &_value)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            return VMResult::STACK_UNDERFLOW;
        else if (pos + sizeof(T) > stack.size())
            return VMResult::STACK_OVERFLOW;

        std::copy((byte *)&_value, (byte *)&_value + sizeof(T), &stack[pos]);
        return VMResult::SUCCESS;
    }

    template <typename T>
    VMResult PushStack(const T &_value)
    {
        VM_PERFORM(WriteStack(0, _value));

        stackPtr += sizeof(T);
        return VMResult::SUCCESS;
    }

    template <typename T>
    VMResult PopStack(T &value)
    {
        VM_PERFORM(ReadStack(-8, value));

        stackPtr -= sizeof(T);
        return VMResult::SUCCESS;
    }
};
