#pragma once
#include "evm.h"
#include <thread>
#include <vector>
#include <optional>
#include "vm.h"

class Thread
{
private:
    VM* vm;

    Memory stack;
    vm_ui64 stackPtr, framePtr;

    std::thread thread;
    ThreadID id;
    bool isAlive;

public:
    const vm_byte* instrPtr;

    Thread(VM* _vm, ThreadID _id, vm_ui64 _stackSize, const vm_byte* _startIP);

    void Start(vm_byte* _globalsArrayPtr, const std::vector<Word>& _args);
    void Join();
    void Run();

    void PushFrame();
    void PopFrame();

    void OffsetSP(vm_i64 _off);
    void PrintStack();

    VM* GetVM() { return vm; }
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

        return *(T*)&stack[_pos];
    }

    template <typename T>
    void WriteStack(vm_i64 _pos, const T& _value)
    {
        if (_pos < 0)
            throw VMError::STACK_UNDERFLOW();
        else if (_pos + sizeof(T) > stack.size())
            throw VMError::STACK_OVERFLOW();

        *(T*)&stack[_pos] = _value;
    }

    void PushStack(Word _value);
    Word PopStack();
};
