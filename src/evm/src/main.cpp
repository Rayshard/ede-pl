#include <iostream>
#include "evm.h"
#include "program.h"
#include "instructions.h"
#include "vm.h"
#include "../build.h"

#ifdef BUILD_DEBUG
bool PRINT_INSTR_BEFORE_EXECUTION = false;
bool PRINT_STACK_AFTER_THREAD_END = false;
bool PRINT_STACK_AFTER_INSTR_EXECUTION = false;
bool PRINT_HEAP_AFTER_PROGRAM_END = false;
#endif

#ifdef BUILD_WITH_TESTS
#include "tests.h"
#endif

#define CLI_FAILURE() assert(false && "CLI Failure!")

int usage(const std::string &_cmd = "")
{
    if (_cmd == "")
    {
        std::cout << "Usage: evm COMMAND [ARGS]...\n\n"
                     "Options:\n"
#ifdef BUILD_DEBUG
                     "   --ibe      Print each instruction before it is executed.\n"
                     "   --sate     Print each thread's stack after the thread ends.\n"
                     "   --saie     Print each thread's stack after an instruction is executed.\n"
                     "   --hape     Print the heap after the program ends.\n"
#endif
                     "\n"
                     "Commands:\n"
                     "   test       Run test suite.\n"
                     "   run        Executes an ede program.\n"
                  << std::endl;
    }
    else if (_cmd == "run")
    {
        std::cout << "Usage: evm run FILEPATH [ARGS]...\n\n"
                     "Options:\n"
                     "  --no-gc         Disables the garbage collector.\n"
                     "\n"
                     "Args:\n"
                     "  FILEPATH        The edeasm file to execute.\n"
                     "  ARGS            List of arguments to pass to the program.\n"
                  << std::endl;
    }
    else
        CLI_FAILURE();

    return 0;
}

int test(const std::vector<std::string> &_args)
{
#ifdef BUILD_WITH_TESTS
    if (_args.size() == 0)
        RUN_TEST_SUITE();
    else
    {
        for (auto &name : _args)
            RUN_TEST(name);
    }
#else
    std::cout << "Tests were not included in build!" << std::endl;
#endif

    return 0;
}

int run(const std::vector<std::string> &_args)
{
    if (_args.empty())
        return usage("run");

    bool runGC = true;
    auto itArg = _args.begin();

    while (itArg != _args.end())
    {
        auto arg = *itArg;
        if (arg[0] != '-')
            break;

        //Add new options here
        if (arg == "--no-gc")
            runGC = false;
        else
        {
            std::cout << "Unknown Option: " << arg << std::endl;
            return usage("run");
        }

        itArg++;
    }

    if (itArg == _args.end())
    {
        std::cout << "Expected file path" << std::endl;
        return usage("run");
    }

    auto filePath = *itArg;
    std::vector<std::string> cmdLineArgs(itArg, _args.end());

    try
    {
        //TODO: first instruction should be CALL MAIN
        //the args for main is the array of cmdline args and it will already
        //be in the place where the paramters are

        Program program = Program::FromFile(filePath);                  //Parse the ede asm file
        auto exitCode = VM(runGC).Run(1024, program, std::move(cmdLineArgs)); //Run

        std::cout << "\nExited with code " << exitCode << "." << std::endl;
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

    size_t iArg = 1;
    std::string arg;

    //Parse options
    while (true)
    {
        arg = _argv[iArg];
        if (arg[0] != '-')
            break;

            //Add new options here
#ifdef BUILD_DEBUG
        if (arg == "--ibe")
            PRINT_INSTR_BEFORE_EXECUTION = true;
        else if (arg == "--saie")
            PRINT_STACK_AFTER_INSTR_EXECUTION = true;
        else if (arg == "--sate")
            PRINT_STACK_AFTER_THREAD_END = true;
        else if (arg == "--hape")
            PRINT_HEAP_AFTER_PROGRAM_END = true;
        else
#endif
        {
            std::cout << "Unknown Option: " << arg << std::endl;
            return usage();
        }

        if (++iArg == _argc)
        {
            std::cout << "Expected Command" << std::endl;
            return usage();
        }
    }

    auto commandsSearch = commands.find(arg);
    if (commandsSearch != commands.end())
        return commandsSearch->second(std::vector<std::string>(_argv + iArg + 1, _argv + _argc));

    std::cout << "Unknown Command: " << arg << std::endl;
    return usage();
}