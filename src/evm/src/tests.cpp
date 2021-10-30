#include "tests.h"
#include "evm.h"
#include "instructions.h"
#include "program.h"
#include "vm.h"
#include <fstream>
#include <string>
#include <sstream>
#include <random>

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

DEFINE_TEST(LLOAD)
{
    std::stringstream stream(
        "   CALL @FUNC 16\n"
        "@FUNC:\n"
        "   PUSHI 123"
        "   SSTORE -16\n"
        "   PUSHI 456"
        "   SSTORE -8\n"
        "   LLOAD 0"
        "   LLOAD 1"
        "   IADD"
        "   EXIT\n");
    Program program = Instructions::ParseStream(stream);

    ASSERT(VM().Run(72, &program[0]) == 579);
}

DEFINE_TEST(LSTORE)
{
    std::stringstream stream(
        "   CALL @FUNC 16\n"
        "@FUNC:\n"
        "   PUSHI 123"
        "   LSTORE 0\n"
        "   PUSHI 456"
        "   LSTORE 1\n"
        "   LLOAD 0"
        "   LLOAD 1"
        "   IADD"
        "   EXIT\n");
    Program program = Instructions::ParseStream(stream);

    ASSERT(VM().Run(72, &program[0]) == 579);
}

DEFINE_TEST(PLOAD)
{
    std::stringstream stream(
        "   PUSHI 123"
        "   PUSHI 456"
        "   CALL @FUNC 0\n"
        "@FUNC:\n"
        "   PLOAD 1"
        "   PLOAD 0"
        "   ISUB"
        "   EXIT\n");
    Program program = Instructions::ParseStream(stream);
    ASSERT(VM().Run(72, &program[0]) == 333);
}

DEFINE_TEST(PSTORE)
{
    std::stringstream stream(
        "   PUSHI 0\n"
        "   PUSHI 0\n"
        "   CALL @FUNC 0\n"
        "   ISUB\n"
        "   EXIT\n"
        "@FUNC:\n"
        "   PUSHI 123\n"
        "   PSTORE 1\n"
        "   PUSHI 456\n"
        "   PSTORE 0\n"
        "   RET\n");
    Program program = Instructions::ParseStream(stream);
    ASSERT(VM().Run(72, &program[0]) == 333);
}

DEFINE_TEST(GLOAD)
{
    std::stringstream stream(
        "   PUSHI 123"
        "   PUSHI 456"
        "   PUSHI 789"
        "   GLOAD 0"
        "   GLOAD 1"
        "   ISUB"
        "   EXIT\n");
    Program program = Instructions::ParseStream(stream);
    ASSERT(VM().Run(72, &program[0]) == 333);
}

DEFINE_TEST(GSTORE)
{
    std::stringstream stream(
        "   PUSHI 0"
        "   PUSHI 0"
        "   PUSHI 123"
        "   PUSHI 456"
        "   GSTORE 1"
        "   GSTORE 0"
        "   ISUB"
        "   EXIT\n");
    Program program = Instructions::ParseStream(stream);
    ASSERT(VM().Run(72, &program[0]) == 333);
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

    ASSERT(Word(VM().Run(64, &program[0])).as_f64 == 579.0);
}

DEFINE_TEST(DSUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 456.0,
        OpCode::PUSH, 123.0,
        OpCode::DSUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_f64 == -333.0);
}

DEFINE_TEST(DMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_f64 == 56088.0);
}

DEFINE_TEST(DDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 20.0,
        OpCode::PUSH, 100.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(Word(VM().Run(64, &program[0])).as_f64 == 5.0);
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
#pragma endregion

#pragma region Branching
DEFINE_TEST(JUMP)
{
    Program program = CreateProgram(
        OpCode::PUSH, (vm_i64)100,
        OpCode::JUMP, (vm_ui64)28,
        OpCode::PUSH, (vm_i64)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (vm_i64)300,
        OpCode::JUMP, (vm_ui64)27);

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
        OpCode::PUSH, (vm_i64)0,
        OpCode::JUMPZ, (vm_ui64)28,
        OpCode::PUSH, (vm_i64)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (vm_i64)5,
        OpCode::JUMPZ, (vm_ui64)18,
        OpCode::PUSH, (vm_i64)300,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 300);
}

DEFINE_TEST(JUMPNZ)
{
    Program program = CreateProgram(
        OpCode::PUSH, (vm_i64)5,
        OpCode::JUMPNZ, (vm_ui64)28,
        OpCode::PUSH, (vm_i64)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (vm_i64)0,
        OpCode::JUMPNZ, (vm_ui64)18,
        OpCode::PUSH, (vm_i64)300,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 300);
}
#pragma endregion

#pragma region System Calls
DEFINE_TEST(SYSCALL_EXIT)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    ASSERT(VM().Run(64, &program[0]) == 100);
}

DEFINE_TEST(SYSCALL_PRINTC)
{
    Program program = CreateProgram(
        OpCode::PUSH, 65ll,
        OpCode::SYSCALL, SysCallCode::PRINTC,
        OpCode::PUSH, 123ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm;
    std::wstringstream stdIO;
    vm.SetStdIO(stdIO.rdbuf(), stdIO.rdbuf());

    ASSERT(vm.Run(16, &program[0]) == 123ll);
    ASSERT(stdIO.str() == L"A");
}

DEFINE_TEST(SYSCALL_MALLOC)
{
    Program program = CreateProgram(
        OpCode::PUSH, 16ull,
        OpCode::SYSCALL, SysCallCode::MALLOC,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm;
    vm_byte *address = (vm_byte *)Word(vm.Run(8, &program[0])).as_ptr;
    ASSERT(vm.GetHeap().IsAllocated(address));
}

DEFINE_TEST(SYSCALL_FREE)
{
    Program program = CreateProgram(
        OpCode::PUSH, 16ull,
        OpCode::SYSCALL, SysCallCode::MALLOC,
        OpCode::SLOAD, -8ll,
        OpCode::SYSCALL, SysCallCode::FREE,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm;
    vm_byte *address = (vm_byte *)Word(vm.Run(16, &program[0])).as_ptr;
    ASSERT(!vm.GetHeap().IsAllocated(address));
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

    ASSERT(Word(VM().Run(64, &program[0])).as_f64 == 579.0);
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
    vm_i64 expectedExitCode;

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

DEFINE_TEST(TEST_HEAP)
{
    std::random_device randDev;
    std::mt19937 randRng(randDev());
    std::vector<vm_byte *> alives;
    Heap heap;

    std::uniform_int_distribution<std::mt19937::result_type> boolGenerator(0, 3);
    std::uniform_int_distribution<std::mt19937::result_type> sizeGenerator(1, 128);

    for (int i = 0; i < 1000; i++)
    {
        if (boolGenerator(randRng) > 0)
        {
            alives.push_back(heap.Alloc(sizeGenerator(randRng)));
        }
        else if(!alives.empty())
        {
            std::uniform_int_distribution<std::mt19937::result_type> aliveIdxPicker(0, alives.size() - 1);
            auto pos = alives.begin() + aliveIdxPicker(randRng);
            auto ptr = *pos;

            heap.Free(ptr);
            alives.erase(pos);
        }

        heap.Print();
        std::cin.get();
    }
    
    heap.AssertHeuristics();
    heap.Print();
}