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

    ASSERT(VM().Run(64, &program[0]) == 579ll);
}

DEFINE_TEST(PUSH)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 123ll);
}

DEFINE_TEST(POP)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456ll,
        OpCode::PUSH, 789ll,
        OpCode::PUSH, 123ll,
        OpCode::POP,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 789ll);
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

    ASSERT(VM().Run(64, &program[0]) == 123ll);
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

    ASSERT(VM().Run(64, &program[0]) == 789ll);
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

    ASSERT(VM().Run(64, &program[0]) == 579ll);
}

DEFINE_TEST(ISUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456ll,
        OpCode::PUSH, 123ll,
        OpCode::ISUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == -333ll);
}

DEFINE_TEST(IMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::IMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 56088ll);
}

DEFINE_TEST(IDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 20ll,
        OpCode::PUSH, 100ll,
        OpCode::IDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 5ll);
}

DEFINE_TEST(DADD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_double == 579.0);
}

DEFINE_TEST(DSUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456.0,
        OpCode::PUSH, 123.0,
        OpCode::DSUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_double == -333.0);
}

DEFINE_TEST(DMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_double == 56088.0);
}

DEFINE_TEST(DDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 20.0,
        OpCode::PUSH, 100.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_double == 5.0);
}

DEFINE_TEST(EQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100.0,
        OpCode::PUSH, 100.0,
        OpCode::EQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 1ull);
}

DEFINE_TEST(NEQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::NEQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 0ull);
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

    ASSERT(VM().Run(64, &program[0]) == 300);
}

DEFINE_TEST(CALL)
{
    std::stringstream stream1("   CALL @FUNC 48\n"
                              "   PUSHI 0\n"
                              "   EXIT\n"
                              "@FUNC:\n"
                              "   PUSHI 1\n"
                              "   EXIT\n");
    Program program1 = Instructions::ParseStream(stream1);

    ASSERT(VM().Run(72, &program1[0]) == 1);

    std::stringstream stream2("   CALL @FUNC 48\n"
                              "   PUSHI 0\n"
                              "   EXIT\n"
                              "@FUNC:\n"
                              "   PUSHI 1\n"
                              "   PUSHI 2\n"
                              "   EXIT\n");
    Program program2 = Instructions::ParseStream(stream2);

    try
    {
        VM().Run(64, &program2[0]);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::STACK_OVERFLOW);
    }
}

DEFINE_TEST(RET)
{
    std::stringstream stream1("   CALL @FUNC 16\n"
                              "   PUSHI 123\n"
                              "   PUSHI 456\n"
                              "   PUSHI 789\n"
                              "   PUSHI 147\n"
                              "   EXIT\n"
                              "@FUNC:\n"
                              "   RET\n");
    Program program1 = Instructions::ParseStream(stream1);

    ASSERT(VM().Run(32, &program1[0]) == 147);

    std::stringstream stream2("   CALL @FUNC 16\n"
                              "   PUSHI 123\n"
                              "   PUSHI 456\n"
                              "   PUSHI 789\n"
                              "   PUSHI 147\n"
                              "   PUSHI 258\n"
                              "   EXIT\n"
                              "@FUNC:\n"
                              "   RET\n");
    Program program2 = Instructions::ParseStream(stream2);

    try
    {
        VM().Run(32, &program2[0]);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::STACK_OVERFLOW);
    }
}

DEFINE_TEST(RETV)
{
    std::stringstream stream("   CALL @FUNC 16\n"
                              "   EXIT\n"
                              "@FUNC:\n"
                              "   PUSHI 123\n"
                              "   RETV\n");
    Program program = Instructions::ParseStream(stream);

    ASSERT(VM().Run(48, &program[0]) == 123);
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

    ASSERT(VM().Run(64, &program[0]) == 300);
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

    ASSERT(VM().Run(64, &program[0]) == 300);
}

DEFINE_TEST(SYSCALL_EXIT)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 100);
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

    ASSERT(VM().Run(64, &program[0]) == 579ll);
}

DEFINE_TEST(I2D)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456.0,
        OpCode::PUSH, 123ll,
        OpCode::I2D,
        OpCode::DADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_double == 579.0);
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

    try
    {
        VM().Run(64, &program[0]);
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

    try
    {
        VM().Run(64, &program[0]);
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

    try
    {
        VM().Run(24, &program[0]);
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

    try
    {
        VM().Run(24, &program[0]);
        ASSERT(false);
    }
    catch (const VMError &e)
    {
        ASSERT(e.GetType() == VMErrorType::STACK_UNDERFLOW);
    }
}

DEFINE_TEST(UNKNOWN_OP_CODE)
{
    Program program;
    Instructions::Insert(program,
                         OpCode::PUSH, 100ll,
                         OpCode::_COUNT, 100ull,
                         OpCode::SYSCALL, SysCallCode::EXIT);

    try
    {
        VM().Run(24, &program[0]);
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

    try
    {
        VM().Run(24, &program[0]);
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

    while (expectedFile >> caseName)
    {
        expectedFile >> expectedExitCode;

        //Get expected output
        std::getline(expectedFile, expectedOutput);
        expectedOutput = expectedOutput.substr(expectedOutput.find_first_of(L'"') + 1);
        expectedOutput = expectedOutput.substr(0, expectedOutput.find_last_of(L'"'));

        std::string filePath = dirPath + std::string(caseName.begin(), caseName.end()) + ".edeasm";
        Program program = Instructions::ParseFile(filePath.c_str());

        VM vm;
        std::wstringstream stdIO;
        vm.SetStdIO(stdIO.rdbuf(), stdIO.rdbuf());

        ASSERT_MSG(vm.Run(64, &program[0]) == expectedExitCode, filePath);
        ASSERT_MSG(stdIO.str() == expectedOutput, filePath);
    }
}