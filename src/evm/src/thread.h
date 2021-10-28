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

    Memory stack;
    vm_ui64 stackPtr, framePtr;

    std::thread thread;
    ThreadID id;
    bool isAlive;

public:
    vm_byte *instrPtr;

    Thread(VM *_vm, ThreadID _id, vm_ui64 _stackSize, vm_byte *_startIP);

    void Start();
    void Join();
    void Run();

    void PushFrame();
    void PopFrame();

    void OffsetSP(vm_i64 _off);
    void PrintStack();

    VM *GetVM() { return vm; }
    ThreadID GetID() { return id; }
    bool IsAlive() { return isAlive; }
    vm_ui64 GetSP() { return stackPtr; }
    vm_ui64 GetFP() { return framePtr; }
    const Memory& GetStack() { return stack; }

    template <typename T>
    T ReadStack(vm_i64 _pos)
    {
        if (_pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (_pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        return *(T *)&stack[_pos];
    }

    template <typename T>
    void WriteStack(vm_i64 _pos, const T &_value)
    {
        if (_pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (_pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        std::copy((vm_byte *)&_value, (vm_byte *)&_value + sizeof(T), &stack[_pos]);
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
