#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>

union Word {
    int64_t as_int;
    double as_double;
    void* as_ptr;
    char bytes[8];
};

#define WORD_SIZE sizeof(Word)

typedef std::vector<char> Program;

enum Instruction
{
    PUSHI,
    PUSHD,
    ADDI,
    SUBI,
    MULI,
    DIVI,
    ADDD,
    POP,
    JUMP,
};

class VM {
    private:
        std::vector<Word> stack;

    public:
        VM(int _stackSize)
        {
            assert(_stackSize % WORD_SIZE == 0);
            stack = std::vector<Word>(_stackSize / WORD_SIZE);
        }

        int Run(Program _program)
        {
            std::cout << _program.data() << std::endl;
            return 0;
        }
};

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
    
    if (!file.read(program.data(), program.size()))
    {
        std::cout << "Unable to run read program file!" << std::endl;
        return -1;
    }

    VM vm(1024);
    return vm.Run(program);
}