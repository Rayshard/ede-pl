#pragma once
#include "evm.h"
#include <mutex>
#include <map>
#include <variant>
#include "program.h"

class Thread;
typedef vm_ui64 ThreadID;

enum class VMErrorType
{
    UNKNOWN_OP_CODE,      // Unable to decode the op code at the instruction pointer
    STACK_OVERFLOW,       // Operation caused stack point to be greater than stack's size
    STACK_UNDERFLOW,      // Operation caused stack point to be less than stack's size
    DIV_BY_ZERO,          // Division by zero occurred
    CANNOT_SPAWN_THREAD,  // Thread could not be spawned
    UNKNOWN_SYSCALL_CODE, // Unable to decode the code for the syscall instruction
    INVALID_FP,           // The frame pointer has been set to a position outside of the stack
    MEMORY_NOT_ALLOCATED, // Attempted to free unallocated memory
    _COUNT
};

class VMError : public std::runtime_error
{
    VMErrorType type;

    VMError(VMErrorType _type, std::string _msg)
        : type(_type), std::runtime_error(_msg) {}

public:
    VMErrorType GetType() const { return type; }

    static VMError UNKNOWN_OP_CODE() { return VMError(VMErrorType::UNKNOWN_OP_CODE, "Unknown op code encountered!"); }
    static VMError STACK_OVERFLOW() { return VMError(VMErrorType::STACK_OVERFLOW, "Stack overflow!"); }
    static VMError STACK_UNDERFLOW() { return VMError(VMErrorType::STACK_UNDERFLOW, "Stack underflow!"); }
    static VMError DIV_BY_ZERO() { return VMError(VMErrorType::DIV_BY_ZERO, "Division by zero!"); }
    static VMError CANNOT_SPAWN_THREAD() { return VMError(VMErrorType::CANNOT_SPAWN_THREAD, "Cannot spawn thread!"); }
    static VMError UNKNOWN_SYSCALL_CODE() { return VMError(VMErrorType::UNKNOWN_SYSCALL_CODE, "Unknown syscall code encountered!"); }
    static VMError INVALID_FP() { return VMError(VMErrorType::INVALID_FP, "Frame pointer has been set to an invalid position!"); }
    static VMError MEMORY_NOT_ALLOCATED() { return VMError(VMErrorType::INVALID_FP, "Attempted to free unallocated memory!"); }
};

typedef std::variant<VMError, vm_i64> VMExitCode;

class VM
{
private:
    bool running;
    std::map<vm_byte *, std::vector<vm_byte>> heap;
    std::map<ThreadID, Thread> threads;
    ThreadID nextThreadID;
    VMExitCode exitCode;
    std::wistream stdInput;
    std::wostream stdOutput;

public:
    std::mutex mutex;

    VM();
    ~VM();

    vm_i64 Run(vm_ui64 _stackSize, vm_byte *_startIP);
    void Quit(VMExitCode _code);

    vm_byte *Malloc(vm_ui64 _amt);
    void Free(vm_byte *_addr);
    bool IsAllocated(vm_byte *_addr);

    ThreadID SpawnThread(vm_ui64 _stackSize, vm_byte *_startIP);
    void JoinThread(vm_ui64 _id);
    void SetStdIO(std::wstreambuf *_in = nullptr, std::wstreambuf *_out = nullptr);

    bool IsRunning() { return running; }
    std::wistream &GetStdIn() { return stdInput; }
    std::wostream &GetStdOut() { return stdOutput; }
};