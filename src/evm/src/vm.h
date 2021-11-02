#pragma once
#include "evm.h"
#include <mutex>
#include <map>
#include <variant>
#include "program.h"
#include "heap.h"

class Thread;
typedef vm_ui64 ThreadID;

enum class VMErrorType
{
    UNKNOWN_OP_CODE,             // Unable to decode the op code at the instruction pointer
    STACK_OVERFLOW,              // Operation caused stack point to be greater than stack's size
    STACK_UNDERFLOW,             // Operation caused stack point to be less than stack's size
    DIV_BY_ZERO,                 // Division by zero occurred
    CANNOT_SPAWN_THREAD,         // Thread could not be spawned
    UNKNOWN_SYSCALL_CODE,        // Unable to decode the code for the syscall instruction
    INVALID_FP,                  // The frame pointer has been set to a position outside of the stack
    MEMORY_NOT_ALLOCATED,        // Attempted to free unallocated memory
    INVALID_THREAD_ID,           // A thread with that id either has never been created or has already died
    CANNOT_FREE_UNALLOCATED_PTR, // Cannot free an unallocated memory pointer
    _COUNT
};

class VMError : public std::runtime_error
{
    VMErrorType type;

    VMError(VMErrorType _type, std::string _msg)
        : type(_type), std::runtime_error(_msg) {}

public:
    VMErrorType GetType() const { return type; }

    static VMError UNKNOWN_OP_CODE(vm_byte _code) { return VMError(VMErrorType::UNKNOWN_OP_CODE, "Unknown op code encountered: [" + std::to_string(_code) + "]!"); }
    static VMError STACK_OVERFLOW() { return VMError(VMErrorType::STACK_OVERFLOW, "Stack overflow!"); }
    static VMError STACK_UNDERFLOW() { return VMError(VMErrorType::STACK_UNDERFLOW, "Stack underflow!"); }
    static VMError DIV_BY_ZERO() { return VMError(VMErrorType::DIV_BY_ZERO, "Division by zero!"); }
    static VMError CANNOT_SPAWN_THREAD() { return VMError(VMErrorType::CANNOT_SPAWN_THREAD, "Cannot spawn thread!"); }
    static VMError UNKNOWN_SYSCALL_CODE(vm_byte _code) { return VMError(VMErrorType::UNKNOWN_SYSCALL_CODE, "Unknown syscall code encountered: [" + std::to_string(_code) + "]!"); }
    static VMError INVALID_FP() { return VMError(VMErrorType::INVALID_FP, "Frame pointer has been set to an invalid position!"); }
    static VMError MEMORY_NOT_ALLOCATED() { return VMError(VMErrorType::INVALID_FP, "Attempted to free unallocated memory!"); }
    static VMError INVALID_THREAD_ID(ThreadID _id) { return VMError(VMErrorType::INVALID_THREAD_ID, "A thread with id [" + std::to_string(_id) + "] does not exist or has already died!"); }
    static VMError CANNOT_FREE_UNALLOCATED_PTR(vm_byte *_ptr) { return VMError(VMErrorType::CANNOT_FREE_UNALLOCATED_PTR, "Cannot free unallocated memory pointer: " + PtrToStr(_ptr)); }
};

typedef std::variant<VMError, vm_i64> VMExitCode;

class VM
{
private:
    Heap heap;
    bool runGC;

    std::map<ThreadID, Thread> threads;
    ThreadID nextThreadID;
    VMExitCode exitCode;
    std::wistream stdInput;
    std::wostream stdOutput;

    bool running;

public:
    std::mutex mutex;

    VM(bool _runGC = false);
    ~VM();

    vm_i64 Run(vm_ui64 _stackSize, vm_byte *_startIP, const std::vector<std::string>& _cmdLineArgs);
    void Quit(VMExitCode _code);

    ThreadID SpawnThread(vm_ui64 _stackSize, vm_byte *_startIP, const std::vector<Word>& _args);
    Thread &GetThread(vm_ui64 _id);
    void SetStdIO(std::wstreambuf *_in = nullptr, std::wstreambuf *_out = nullptr);

    bool IsRunning() { return running; }
    std::wistream &GetStdIn() { return stdInput; }
    std::wostream &GetStdOut() { return stdOutput; }
    Heap &GetHeap() { return heap; }
};