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
        VMResult result = vm.Run(64);

        if (result == VMResult::SUCCESS)
        {
            std::cout << "Exited with code " << vm.GetExitCode() << "." << std::endl;
            return vm.GetExitCode();
        }

        std::cout << GetVMResultString(result) << std::endl;
        return -1;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}