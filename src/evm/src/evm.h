#pragma once

#include <cstdint>
#include <assert.h>

typedef uint8_t byte;

union Word
{
    int64_t as_int;
    uint64_t as_uint;
    double as_double;
    void *as_ptr;
    byte bytes[sizeof(int64_t)];

    Word() : as_int(0) {}
    Word(int64_t _i) : as_int(_i) {}
    Word(uint64_t _ui) : as_uint(_ui) {}
    Word(double _d) : as_double(_d) {}
    Word(void *_p) : as_ptr(_p) {}
};

#define WORD_SIZE sizeof(Word)
static_assert(WORD_SIZE == sizeof(int64_t), "Word is not the right size!");

enum class VMResult
{
    SUCCESS,              // VM ran program with no errors
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

constexpr const char *GetVMResultString(VMResult _result)
{
    switch (_result)
    {
    case VMResult::SUCCESS:
        return "";
    case VMResult::IP_OUT_OF_BOUNDS:
        return "Instruction pointer out of bounds!";
    case VMResult::IP_OVERFLOW:
        return "Instruction pointer overflow!";
    case VMResult::UNKNOWN_OP_CODE:
        return "Unknown op code encountered!";
    case VMResult::STACK_OVERFLOW:
        return "Stack overflow!";
    case VMResult::STACK_UNDERFLOW:
        return "Stack underflow!";
    case VMResult::DIV_BY_ZERO:
        return "Division by zero!";
    case VMResult::CANNOT_SPAWN_THREAD:
        return "Cannot spawn thread!";
    case VMResult::UNKNOWN_SYSCALL_CODE:
        return "Unknown syscall code encountered!";
    default:
        assert(false && "Case not handled");
    }

    return "";
}

#define VM_PERFORM(x)                         \
    (                                         \
        {                                     \
            auto _result = x;                 \
            if (_result != VMResult::SUCCESS) \
                return _result;               \
        })

class VM;
class Thread;