#pragma once
#include "evm.h"
#include <mutex>
#include <map>
#include "program.h"

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
    VMError IP_OUT_OF_BOUNDS() { return VMError(VMErrorType::IP_OUT_OF_BOUNDS, "Instruction pointer out of bounds!"); }
    VMError IP_OVERFLOW() { return VMError(VMErrorType::IP_OVERFLOW, "Instruction pointer overflow!"); }
    VMError UNKNOWN_OP_CODE() { return VMError(VMErrorType::UNKNOWN_OP_CODE, "Unknown op code encountered!"); }
    VMError STACK_OVERFLOW() { return VMError(VMErrorType::STACK_OVERFLOW, "Stack overflow!"); }
    VMError STACK_UNDERFLOW() { return VMError(VMErrorType::STACK_UNDERFLOW, "Stack underflow!"); }
    VMError DIV_BY_ZERO() { return VMError(VMErrorType::DIV_BY_ZERO, "Division by zero!"); }
    VMError CANNOT_SPAWN_THREAD() { return VMError(VMErrorType::CANNOT_SPAWN_THREAD, "Cannot spawn thread!"); }
    VMError UNKNOWN_SYSCALL_CODE() { return VMError(VMErrorType::UNKNOWN_SYSCALL_CODE, "Unknown syscall code encountered!"); }
};

class VM
{
private:
    Program program;
    bool running;
    std::map<byte *, std::vector<byte>> heap;
    std::map<size_t, Thread> threads;
    size_t nextThreadID;
    struct
    {
        int64_t code;
        VMResult result;
    } exitInfo;

public:
    std::mutex mutex;

    VM(Program &&_program);
    ~VM();

    VMResult Run(size_t _stackSize);
    void Quit(VMResult _res, int64_t _code);

    VMResult SpawnThread(size_t _stackSize, size_t _startIP, size_t &_id);
    void JoinThread(size_t _id);

    const Program &GetProgram() { return program; }
    int64_t GetExitCode() { return exitInfo.code; }
    bool IsRunning() { return running; }
};