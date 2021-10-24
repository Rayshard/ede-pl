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
typedef std::vector<byte> Program;

class VM;
class Thread;

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

extern const char *VMResultStrings[(size_t)VMResult::_COUNT];

#define VM_PERFORM(x)                         \
    (                                         \
        {                                     \
            auto _result = x;                 \
            if (_result != VMResult::SUCCESS) \
                return _result;               \
        })

namespace Instructions
{
    enum class OpCode : byte
    {
        OP_CODES,
        _COUNT
    };

    extern const size_t Sizes[(size_t)OpCode::_COUNT];

    typedef VMResult (*ExecutionFunc)(Thread *);
    extern ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];

    void Init();

    template <class type>
    void Insert(Program &_prog, type _value) { _prog.insert(_prog.end(), (char *)&_value, (char *)&_value + sizeof(_value)); }

    template <typename Arg1, typename... Rest>
    void Insert(Program &_prog, Arg1 _arg1, Rest const &..._rest)
    {
        Insert(_prog, _arg1);
        Insert(_prog, _rest...);
    }

    template <typename Arg1, typename... Rest>
    Program CreateProgram(Arg1 _arg1, Rest const &..._rest)
    {
        Program program = Program();
        Insert(program, _arg1);
        Insert(program, _rest...);
        return program;
    }
}

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

    VMResult PushStack(Word _word);
    VMResult PopStack(Word &word);
    void PrintStack();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
    bool IsAlive() { return isAlive; }
    Word GetStackTop() { return *(Word*)&stack[stackPtr]; }
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
