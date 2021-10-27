#pragma once
#include "evm.h"
#include <mutex>
#include <map>
#include <variant>
#include "program.h"

class Thread;

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

typedef std::variant<VMError, int64_t> VMExitCode;

class VM
{
private:
    bool running;
    std::map<byte *, std::vector<byte>> heap;
    std::map<size_t, Thread> threads;
    size_t nextThreadID;
    VMExitCode exitCode;
    std::wistream stdInput;
    std::wostream stdOutput;

public:
    std::mutex mutex;

    VM();
    ~VM();

    int64_t Run(size_t _stackSize, byte *_startIP);
    void Quit(VMExitCode _code);

    byte *Malloc(size_t _amt);
    void Free(byte *_addr);
    bool IsAllocated(byte *_addr);

    size_t SpawnThread(size_t _stackSize, byte *_startIP);
    void JoinThread(size_t _id);
    void SetStdIO(std::wstreambuf *_in = nullptr, std::wstreambuf *_out = nullptr);

    bool IsRunning() { return running; }
    std::wistream &GetStdIn() { return stdInput; }
    std::wostream &GetStdOut() { return stdOutput; }
};