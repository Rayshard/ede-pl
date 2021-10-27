#include <iostream>
#include "evm.h"
#include "program.h"
#include "instructions.h"
#include "vm.h"
#include "../build.h"

#ifdef BUILD_WITH_TESTS
#include "tests.h"
#endif

#define CLI_FAILURE() assert(false && "CLI Failure!")

int usage(const std::string &_cmd = "")
{
    if (_cmd == "")
    {
        std::cout << "Usage: evm COMMAND [ARGS]...\n\n"
                     "Commands:\n"
                     "   test       Run test suite.\n"
                     "   run        Executes an ede program.\n"
                  << std::endl;
    }
    else if (_cmd == "run")
    {
        std::cout << "Usage: evm run FILEPATH\n\n"
                     "Args:\n"
                     "  FILEPATH        The edeasm file to execute.\n"
                  << std::endl;
    }
    else
        CLI_FAILURE();

    return 0;
}

int test(const std::vector<std::string> &_args)
{
#ifdef BUILD_WITH_TESTS
    RUN_TEST_SUITE();
#else
    std::cout << "Tests were not included in build!" << std::endl;
#endif

    return 0;
}

int run(const std::vector<std::string> &_args)
{
    if (_args.size() != 1)
        return usage("run");

    try
    {
        //Parse the ede asm file
        Program program = Instructions::ParseFile(_args[0]);
        
        //Run
        auto exitCode = VM().Run(64, &program[0]);

        std::cout << "Exited with code " << exitCode << "." << std::endl;
        return exitCode;
    }
    catch (const std::runtime_error &e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
}

int main(int _argc, char *_argv[])
{
    typedef int (*CommandFunc)(const std::vector<std::string> &);
    static std::map<std::string, CommandFunc> commands = {
        {"test", &test},
        {"run", &run},
    };

    Instructions::Init();

    if (_argc == 1)
        return usage();

    auto commandsSearch = commands.find(_argv[1]);
    if (commandsSearch != commands.end())
        return commandsSearch->second(std::vector<std::string>(_argv + 2, _argv + 2 + _argc - 2));

    std::cout << "Unknown Command: " << _argv[1] << std::endl;
    return usage();
}