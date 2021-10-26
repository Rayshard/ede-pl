#pragma once
#include "evm.h"
#include <mutex>
#include <map>
#include <variant>
#include "program.h"

class Thread;

enum class VMErrorType
{
    IP_OUT_OF_BOUNDS,     // The instruction pointer was out of bounds of the program
    IP_OVERFLOW,          // Decoding the instruction at the instruction pointer would overflow from the program
    UNKNOWN_OP_CODE,      // Unable to decode the op code at the instruction pointer
    STACK_OVERFLOW,       // Operation caused stack point to be greater than stack's size
    STACK_UNDERFLOW,      // Operation caused stack point to be less than stack's size
    DIV_BY_ZERO,          // Division by zero occurred
    CANNOT_SPAWN_THREAD,  // Thread could not be spawned
    UNKNOWN_SYSCALL_CODE, // Unable to decode the code for the syscall instruction
    _COUNT
};

class VMError : public std::runtime_error
{
    VMErrorType type;

    VMError(VMErrorType _type, std::string _msg)
        : type(_type), std::runtime_error(_msg) {}

public:
    VMErrorType GetType() const { return type; }

    static VMError IP_OUT_OF_BOUNDS() { return VMError(VMErrorType::IP_OUT_OF_BOUNDS, "Instruction pointer out of bounds!"); }
    static VMError IP_OVERFLOW() { return VMError(VMErrorType::IP_OVERFLOW, "Instruction pointer overflow!"); }
    static VMError UNKNOWN_OP_CODE() { return VMError(VMErrorType::UNKNOWN_OP_CODE, "Unknown op code encountered!"); }
    static VMError STACK_OVERFLOW() { return VMError(VMErrorType::STACK_OVERFLOW, "Stack overflow!"); }
    static VMError STACK_UNDERFLOW() { return VMError(VMErrorType::STACK_UNDERFLOW, "Stack underflow!"); }
    static VMError DIV_BY_ZERO() { return VMError(VMErrorType::DIV_BY_ZERO, "Division by zero!"); }
    static VMError CANNOT_SPAWN_THREAD() { return VMError(VMErrorType::CANNOT_SPAWN_THREAD, "Cannot spawn thread!"); }
    static VMError UNKNOWN_SYSCALL_CODE() { return VMError(VMErrorType::UNKNOWN_SYSCALL_CODE, "Unknown syscall code encountered!"); }
};

typedef std::variant<VMError, int64_t> VMExitCode;

class VM
{
private:
    Program program;
    bool running;
    std::map<byte *, std::vector<byte>> heap;
    std::map<size_t, Thread> threads;
    size_t nextThreadID;
    VMExitCode exitCode;
    std::wistream stdInput;
    std::wostream stdOutput;

public:
    std::mutex mutex;

    VM(Program &&_program);
    ~VM();

    int64_t Run(size_t _stackSize);
    void Quit(VMExitCode _code);

    size_t SpawnThread(size_t _stackSize, size_t _startIP);
    void JoinThread(size_t _id);
    void SetStdIO(std::wstreambuf *_in = nullptr, std::wstreambuf *_out = nullptr);

    const Program &GetProgram() { return program; }
    bool IsRunning() { return running; }
    std::wistream &GetStdIn() { return stdInput; }
    std::wostream &GetStdOut() { return stdOutput; }
};