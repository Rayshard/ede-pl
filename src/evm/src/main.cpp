#include "evm.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    assert(argc == 2 && "Expected path to Ede program file");
    Instructions::Init();

    //Open file for reading
    std::ifstream file(argv[1], std::ios::ate); //The second arg tells the stream to open and seek to the end of the file
    if (!file.is_open())
    {
        std::cout << "Unable to open program file!" << std::endl;
        return -1;
    }

    //Read the program file
    // Program program(file.tellg());
    // file.seekg(0, std::ios::beg);

    // if (!file.read((char *)program.data(), program.size()))
    // {
    //     std::cout << "Unable to read program file!" << std::endl;
    //     return -1;
    // }

    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::PUSH, (int64_t)456,
        Instructions::OpCode::IADD,
        Instructions::OpCode::PUSH, (int64_t)15,
        Instructions::OpCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    if (result == VMResult::SUCCESS)
    {
        std::cout << "Exited with code " << vm.GetExitCode() << "." << std::endl;
        return vm.GetExitCode();
    }

    std::cout << VMResultStrings[(size_t)result] << std::endl;
    return -1;
}