#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <optional>
#include <map>
#include <mutex>
#include <assert.h>

typedef uint8_t byte;
typedef std::vector<byte> Program;

class VM;
class Thread;
enum class VMResult;

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

namespace Instructions
{
    enum class OpCode : byte
    {
        NOOP,
        PUSH,
        POP,
        IADD,
        ISUB,
        IMUL,
        IDIV,
        DADD,
        DSUB,
        DMUL,
        DDIV,
        SLOAD,
        SSTORE,
        EQ,
        NEQ,
        JUMP,
        JUMPZ,
        JUMPNZ,
        SYSCALL,
        _COUNT
    };

    enum class SysCallCode : byte
    {
        EXIT,
        PRINTC,
        PRINTS,
        MALLOC,
        FREE,
        _COUNT
    };

#define OP_CODE_SIZE sizeof(Instructions::OpCode)
#define SYSCALL_CODE_SIZE sizeof(Instructions::SysCallCode)

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

    constexpr size_t GetSize(OpCode _opCode)
    {
        switch (_opCode)
        {
        case OpCode::NOOP:
            return OP_CODE_SIZE;
        case OpCode::PUSH:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::POP:
            return OP_CODE_SIZE;
        case OpCode::IADD:
            return OP_CODE_SIZE;
        case OpCode::ISUB:
            return OP_CODE_SIZE;
        case OpCode::IMUL:
            return OP_CODE_SIZE;
        case OpCode::IDIV:
            return OP_CODE_SIZE;
        case OpCode::DADD:
            return OP_CODE_SIZE;
        case OpCode::DSUB:
            return OP_CODE_SIZE;
        case OpCode::DMUL:
            return OP_CODE_SIZE;
        case OpCode::DDIV:
            return OP_CODE_SIZE;
        case OpCode::JUMP:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::JUMPNZ:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::JUMPZ:
            return OP_CODE_SIZE + WORD_SIZE;
        case OpCode::SYSCALL:
            return OP_CODE_SIZE + SYSCALL_CODE_SIZE;
        case OpCode::EQ:
            return OP_CODE_SIZE;
        case OpCode::NEQ:
            return OP_CODE_SIZE;
        case OpCode::SLOAD:
            return OP_CODE_SIZE + sizeof(int64_t);
        case OpCode::SSTORE:
            return OP_CODE_SIZE + sizeof(int64_t);
        default:
            assert(false && "Case not handled");
        }

        return 0;
    }
}

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

    void PrintStack();

    VM *GetVM() { return vm; }
    size_t GetID() { return id; }
    bool IsAlive() { return isAlive; }

    ///Reads the stack relative to the stack pointer
    template <typename T>
    VMResult ReadStack(int64_t _offset, T &_value)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            return VMResult::STACK_UNDERFLOW;
        else if (pos + sizeof(T) > stack.size())
            return VMResult::STACK_OVERFLOW;

        _value = *(T *)&stack[pos];
        return VMResult::SUCCESS;
    }

    ///Writes to the stack relative to the stack pointer
    template <typename T>
    VMResult WriteStack(size_t _offset, const T &_value)
    {
        int64_t pos = stackPtr + _offset;
        if (pos < 0)
            return VMResult::STACK_UNDERFLOW;
        else if (pos + sizeof(T) > stack.size())
            return VMResult::STACK_OVERFLOW;

        std::copy((byte *)&_value, (byte *)&_value + sizeof(T), &stack[pos]);
        return VMResult::SUCCESS;
    }

    template <typename T>
    VMResult PushStack(const T &_value)
    {
        VM_PERFORM(WriteStack(0, _value));

        stackPtr += sizeof(T);
        return VMResult::SUCCESS;
    }

    template <typename T>
    VMResult PopStack(T &value)
    {
        VM_PERFORM(ReadStack(-8, value));

        stackPtr -= sizeof(T);
        return VMResult::SUCCESS;
    }
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
