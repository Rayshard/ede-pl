#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <optional>
#include <map>
#include <mutex>
#include <assert.h>
#include "template.h"

typedef uint8_t byte;

enum class OpCode : byte
{
    OP_CODES,
    _COUNT
};

unsigned long long InstructionSizes[] = {
    INSTRUCTION_SIZES};

static_assert(sizeof(InstructionSizes) / sizeof(InstructionSizes[0]) == (unsigned long long)OpCode::_COUNT);

union Word
{
    int64_t as_int;
    uint64_t as_uint;
    double as_double;
    void *as_ptr;
    byte bytes[WORD_SIZE];

    Word() : as_int(0) {}
    Word(int64_t _i) : as_int(_i) {}
    Word(uint64_t _ui) : as_uint(_ui) {}
    Word(double _d) : as_double(_d) {}
    Word(void *_p) : as_ptr(_p) {}
};

typedef std::vector<byte> Program;

enum class VMResult
{
    SUCCESS,             // VM ran program with no errors
    IP_OUT_OF_BOUNDS,    // The instruction pointer was out of bounds of the program
    IP_OVERFLOW,         // Decoding the instruction at the instruction pointer would overflow from the program
    UNKNOWN_OP_CODE,     // Unable to decode the op code at the instruction pointer
    STACK_OVERFLOW,      // Operation caused stack point to be greater than stack's size
    STACK_UNDERFLOW,     // Operation caused stack point to be less than stack's size
    DIV_BY_ZERO,         // Division by zero occurred
    CANNOT_SPAWN_THREAD, // Thread could not be spawned
    _COUNT
};

const char *VMResultStrings[] = {
    "",                                   //SUCCESS
    "Instruction pointer out of bounds!", //IP_OUT_OF_BOUNDS
    "Instruction pointer overflow!",      //IP_OVERFLOW
    "Unknown op code encountered!",       //UNKNOWN_OP_CODE
    "Stack overflow!",                    //STACK_OVERFLOW
    "Stack underflow!",                   //STACK_UNDERFLOW
    "Division by zero!",                  //DIV_BY_ZERO
    "Cannot spawn thread!",               //CANNOT_SPAWN_THREAD
};

static_assert(sizeof(VMResultStrings) / sizeof(VMResultStrings[0]) == (size_t)VMResult::_COUNT);

#define VM_PERFORM(x)                         \
    (                                         \
        {                                     \
            auto _result = x;                 \
            if (_result != VMResult::SUCCESS) \
                return _result;               \
        })

class VM;

class Thread
{
private:
    VM *vm;
    size_t id;
    std::vector<byte> stack;
    size_t stackPtr;

    std::thread thread;
    std::optional<VMResult> execResult;

public:
    size_t instrPtr;

    Thread(VM *_vm, size_t _id, size_t _stackSize, size_t _startIP);

    void Start();
    void Join();
    VMResult Run();
    
    VMResult PushStack(Word _word);
    VMResult PopStack(Word &word);
    void PrintStack();

    bool IsAlive();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
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
        int64_t code = 0;
        VMResult result = VMResult::SUCCESS;
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