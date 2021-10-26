#include "tests.h"
#include "evm.h"
#include "instructions.h"
#include "program.h"
#include "vm.h"
#include <fstream>
#include <string>
#include <sstream>

INIT_TEST_SUITE();

using Instructions::OpCode, Instructions::SysCallCode;

DEFINE_TEST(NOOP)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::NOOP,
        OpCode::PUSH, 456ll,
        OpCode::NOOP,
        OpCode::IADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 579ll);
}

DEFINE_TEST(PUSH)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 123ll);
}

DEFINE_TEST(POP)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456ll,
        OpCode::PUSH, 789ll,
        OpCode::PUSH, 123ll,
        OpCode::POP,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 789ll);
}

#pragma region Instructions

#pragma region Loads and Stores
DEFINE_TEST(SLOAD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::SLOAD, -16ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 123ll);
}

DEFINE_TEST(SSTORE)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::PUSH, 789ll,
        OpCode::SSTORE, -16ll,
        OpCode::POP,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 789ll);
}
#pragma endregion

#pragma region Binops
DEFINE_TEST(IADD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::IADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 579ll);
}

DEFINE_TEST(ISUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456ll,
        OpCode::PUSH, 123ll,
        OpCode::ISUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == -333ll);
}

DEFINE_TEST(IMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::IMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 56088ll);
}

DEFINE_TEST(IDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 20ll,
        OpCode::PUSH, 100ll,
        OpCode::IDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 5ll);
}

DEFINE_TEST(DADD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(Word(vm.Run(64)).as_double == 579.0);
}

DEFINE_TEST(DSUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456.0,
        OpCode::PUSH, 123.0,
        OpCode::DSUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(Word(vm.Run(64)).as_double == -333.0);
}

DEFINE_TEST(DMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(Word(vm.Run(64)).as_double == 56088.0);
}

DEFINE_TEST(DDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 20.0,
        OpCode::PUSH, 100.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(Word(vm.Run(64)).as_double == 5.0);
}

DEFINE_TEST(EQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100.0,
        OpCode::PUSH, 100.0,
        OpCode::EQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 1ull);
}

DEFINE_TEST(NEQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::NEQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 0ull);
}

DEFINE_TEST(JUMP)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::JUMP, (int64_t)28,
        OpCode::PUSH, (int64_t)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (int64_t)300,
        OpCode::JUMP, (int64_t)27);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 300);
}

DEFINE_TEST(JUMPZ)
{
    Program program = CreateProgram(
        OpCode::PUSH, (int64_t)0,
        OpCode::JUMPZ, (int64_t)28,
        OpCode::PUSH, (int64_t)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (int64_t)5,
        OpCode::JUMPZ, (int64_t)18,
        OpCode::PUSH, (int64_t)300,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 300);
}

DEFINE_TEST(JUMPNZ)
{
    Program program = CreateProgram(
        OpCode::PUSH, (int64_t)5,
        OpCode::JUMPNZ, (int64_t)28,
        OpCode::PUSH, (int64_t)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (int64_t)0,
        OpCode::JUMPNZ, (int64_t)18,
        OpCode::PUSH, (int64_t)300,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 300);
}

DEFINE_TEST(SYSCALL_EXIT)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 100);
}
#pragma endregion

#pragma region Converters
DEFINE_TEST(D2I)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456.0,
        OpCode::D2I,
        OpCode::IADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(vm.Run(64) == 579ll);
}

DEFINE_TEST(I2D)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456.0,
        OpCode::PUSH, 123ll,
        OpCode::I2D,
        OpCode::DADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    ASSERT(Word(vm.Run(64)).as_double == 579.0);
}
#pragma endregion

#pragma endregion

#pragma region VMErrors
DEFINE_TEST(IDIV_DIV_BY_ZERO)
{
    Program program = CreateProgram(
        OpCode::PUSH, 0ll,
        OpCode::PUSH, 100ll,
        OpCode::IDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(64);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::DIV_BY_ZERO);
    }
}

DEFINE_TEST(DDIV_DIV_BY_ZERO)
{
    Program program = CreateProgram(
        OpCode::PUSH, 0.0,
        OpCode::PUSH, 100.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(64);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::DIV_BY_ZERO);
    }
}

DEFINE_TEST(STACK_OVERFLOW)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::STACK_OVERFLOW);
    }
}

DEFINE_TEST(STACK_UNDERFLOW)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::POP,
        OpCode::POP,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::STACK_UNDERFLOW);
    }
}

DEFINE_TEST(IP_OVERFLOW)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::IP_OVERFLOW);
    }
}

DEFINE_TEST(IP_OUT_OF_BOUNDS)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::JUMP, 100ull,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::IP_OUT_OF_BOUNDS);
    }
}

DEFINE_TEST(UNKNOWN_OP_CODE)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::_COUNT, 100ull,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::UNKNOWN_OP_CODE);
    }
}

DEFINE_TEST(UNKNOWN_SYSCALL_CODE)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::SYSCALL, SysCallCode::_COUNT);

    VM vm(std::move(program));

    try
    {
        vm.Run(24);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::UNKNOWN_SYSCALL_CODE);
    }
}
#pragma endregion

DEFINE_TEST(TEST_FILES)
{
    std::string dirPath = "tests/evm/";
    std::wifstream expectedFile(dirPath + "expected.txt");
    ASSERT(expectedFile.is_open());

    //Skip Header
    std::wstring line;
    std::getline(expectedFile, line);
    std::getline(expectedFile, line);

    std::wstring caseName, expectedOutput;
    int64_t expectedExitCode;

    while(expectedFile >> caseName)
    {
        expectedFile >> expectedExitCode;

        //Get expected output
        std::getline(expectedFile, expectedOutput);
        expectedOutput = expectedOutput.substr(expectedOutput.find_first_of(L'"') + 1);
        expectedOutput = expectedOutput.substr(0, expectedOutput.find_last_of(L'"'));


        std::string filePath = dirPath + std::string(caseName.begin(), caseName.end()) + ".edeasm";
        Program program = Instructions::ParseEdeASM(filePath.c_str());
        VM vm(std::move(program));
        
        std::wstringstream stdIO;
        vm.SetStdIO(stdOut.rdbuf(), stdOut.rdbuf());

        ASSERT_MSG(vm.Run(64) == expectedExitCode, filePath);
        ASSERT_MSG(stdIO.str() == expectedOutput, filePath);
    }
}

int main()
{
    Instructions::Init();
    RUN_TEST_SUITE();
    return 0;
}