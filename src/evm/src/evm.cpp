#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include "template.h"

enum class OpCode
{
    OP_CODES,
    _COUNT
};

unsigned long long InstructionSizes[] = {
    INSTRUCTION_SIZES
};

static_assert(sizeof(InstructionSizes) / sizeof(InstructionSizes[0]) == (unsigned long long)OpCode::_COUNT);

union Word
{
    int64_t as_int;
    double as_double;
    void *as_ptr;
    uint8_t bytes[WORD_SIZE];

    Word() : as_int(0) {}
    Word(int _i) : as_int(_i) {}
    Word(double _d) : as_double(_d) {}
    Word(void *_p) : as_ptr(_p) {}
};

typedef std::vector<uint8_t> Program;

enum class VMResult
{
    SUCCESS,          // VM ran program with no errors
    IP_OUT_OF_BOUNDS, // The instruction pointer was out of bounds of the program
    IP_OVERFLOW,      // Decoding the instruction at the instruction pointer would overflow from the program
    UNKNOWN_OP_CODE,  // Unable to decode the op code at the instruction pointer
    STACK_OVERFLOW,   // Operation caused stack point to be greater than stack's size
    STACK_UNDERFLOW,  // Operation caused stack point to be less than stack's size
    DIV_BY_ZERO,      // Division by zero occurred
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
};

static_assert(sizeof(VMResultStrings) / sizeof(VMResultStrings[0]) == (size_t)VMResult::_COUNT);

#define VM_PERFORM(x)                         \
    (                                         \
        {                                     \
            auto _result = x;                 \
            if (_result != VMResult::SUCCESS) \
                return _result;               \
        })

class VM
{
private:
    std::vector<uint8_t> stack;
    size_t stack_ptr;
    int64_t exitCode;

public:
    VM(int _stackSize) : stack_ptr(0), exitCode(0)
    {
        assert(_stackSize % WORD_SIZE == 0);
        stack = std::vector<uint8_t>(_stackSize);
    }

    VMResult Run(Program _program)
    {
        size_t ip = 0;

        while (true)
        {
            if (ip < 0 || ip >= _program.size())
                return VMResult::IP_OUT_OF_BOUNDS;

            uint8_t opcode = _program[ip];
            if (opcode >= (size_t)OpCode::_COUNT)
                return VMResult::UNKNOWN_OP_CODE;
            else if (ip + InstructionSizes[opcode] > _program.size())
                return VMResult::IP_OVERFLOW;

            switch ((OpCode)opcode)
            {
            case OpCode::PUSH:
            {
                Word word = *(Word *)&_program[ip + OP_CODE_SIZE];
                std::cout << "PUSH " << word.as_int << "|" << word.as_double << std::endl;

                VM_PERFORM(PushStack(word));
            }
            break;
            case OpCode::ADDI:
            {
                std::cout << "ADDI" << std::endl;

                Word left, right, sum;
                VM_PERFORM(PopStack(right));
                VM_PERFORM(PopStack(left));

                sum.as_int = left.as_int + right.as_int;
                VM_PERFORM(PushStack(sum));
            }
            break;
            case OpCode::SUBI:
            {
                std::cout << "SUBI" << std::endl;

                Word left, right, sum;
                VM_PERFORM(PopStack(right));
                VM_PERFORM(PopStack(left));

                sum.as_int = left.as_int - right.as_int;
                VM_PERFORM(PushStack(sum));
            }
            break;
            case OpCode::MULI:
            {
                std::cout << "MULI" << std::endl;

                Word left, right, sum;
                VM_PERFORM(PopStack(right));
                VM_PERFORM(PopStack(left));

                sum.as_int = left.as_int * right.as_int;
                VM_PERFORM(PushStack(sum));
            }
            break;
            case OpCode::DIVI:
            {
                std::cout << "DIVI" << std::endl;

                Word left, right, sum;
                VM_PERFORM(PopStack(right));
                if (right.as_int == 0)
                    return VMResult::DIV_BY_ZERO;

                VM_PERFORM(PopStack(left));

                sum.as_int = left.as_int / right.as_int;
                VM_PERFORM(PushStack(sum));
            }
            break;
            case OpCode::POP:
            {
                std::cout << "POP" << std::endl;

                Word trash;
                VM_PERFORM(PopStack(trash));
            }
            break;
            case OpCode::EXIT:
            {
                std::cout << "EXIT" << std::endl;

                VM_PERFORM(PopStack(*(Word *)&exitCode));
                return VMResult::SUCCESS;
            }
            break;
            default:
                return VMResult::UNKNOWN_OP_CODE;
            }

            ip += InstructionSizes[opcode];
        }

        return VMResult::SUCCESS;
    }

    VMResult PushStack(Word _word)
    {
        if (stack_ptr + WORD_SIZE > stack.size())
            return VMResult::STACK_OVERFLOW;

        std::copy(_word.bytes, _word.bytes + WORD_SIZE, &stack[stack_ptr]);
        stack_ptr += WORD_SIZE;
        return VMResult::SUCCESS;
    }

    VMResult PopStack(Word &word)
    {
        if (stack_ptr < WORD_SIZE)
            return VMResult::STACK_UNDERFLOW;

        word = *(Word *)&stack[stack_ptr - WORD_SIZE];
        stack_ptr -= WORD_SIZE;
        return VMResult::SUCCESS;
    }

    void PrintStack()
    {
        std::cout << std::string(90, '=') << std::endl;

        for (size_t i = 0; i < stack.size(); i += WORD_SIZE)
        {
            std::cout << reinterpret_cast<void *>(&stack[i]);
            std::cout << ":\t" << ((Word *)&stack[i])->as_int;
            std::cout << "\t" << ((Word *)&stack[i])->as_double;

            if (i + WORD_SIZE == stack_ptr)
                std::cout << "\t\t<-------";

            std::cout << std::endl;
        }

        std::cout << std::string(90, '=') << std::endl;
    }

    int64_t GetExitCode() { return exitCode; }
};

template <class type>
void Insert(Program &_prog, type _value)
{
    _prog.insert(_prog.end(), (char *)&_value, (char *)&_value + sizeof(_value));
}

int main(int argc, char *argv[])
{
    assert(argc == 2 && "Expected path to Ede program file");

    //Open file for reading
    std::ifstream file(argv[1], std::ios::ate); //The second arg tells the stream to open and seek to the end of the file
    if (!file.is_open())
    {
        std::cout << "Unable to open program file!" << std::endl;
        return -1;
    }

    //Read the program file
    Program program(file.tellg());
    file.seekg(0, std::ios::beg);

    if (!file.read((char *)program.data(), program.size()))
    {
        std::cout << "Unable to read program file!" << std::endl;
        return -1;
    }

    // Program program = Program();

    // Insert(program, OpCode::PUSH);
    // Insert(program, (int64_t)123);
    // Insert(program, OpCode::PUSH);
    // Insert(program, (int64_t)456);
    // Insert(program, OpCode::ADDI);
    // Insert(program, OpCode::PUSH);
    // Insert(program, (int64_t)15);
    // Insert(program, OpCode::EXIT);

    VM vm(64);
    VMResult result = vm.Run(program);

    if (result == VMResult::SUCCESS)
    {
        //vm.PrintStack();
        std::cout << "Exited with code " << vm.GetExitCode() << "." << std::endl;
        return vm.GetExitCode();
    }

    std::cout << VMResultStrings[(size_t)result] << std::endl;
    return -1;
}