#include <iostream>
#include "evm.h"
#include "program.h"
#include "instructions.h"
#include "vm.h"

int main(int argc, char *argv[])
{
    assert(argc == 2 && "Expected path to Ede asm file!");
    Instructions::Init();

    try
    {
        //Parse the ede asm file
        Program program = Instructions::ParseEdeASM(argv[1]);
        
        //Run
        VM vm(std::move(program));
        auto exitCode = vm.Run(64);

        std::cout << "Exited with code " << exitCode << "." << std::endl;
        return exitCode;
    }
    catch (const std::runtime_error &e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
}