#include "tests.h"
#include "evm.h"

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
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 579ll);
}

DEFINE_TEST(PUSH)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 123ll);
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
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 789ll);
}

DEFINE_TEST(SLOAD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::SLOAD, -16ll,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 123ll);
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
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 789ll);
}

DEFINE_TEST(IADD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::IADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 579ll);
}

DEFINE_TEST(ISUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::ISUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == -333ll);
}

DEFINE_TEST(IMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123ll,
        OpCode::PUSH, 456ll,
        OpCode::IMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 56088ll);
}

DEFINE_TEST(IDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 20ll,
        OpCode::IDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 5ll);
}

DEFINE_TEST(IDIV_DIV_BY_ZERO)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 0ll,
        OpCode::IDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::DIV_BY_ZERO);
}

DEFINE_TEST(DADD)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DADD,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 579.0);
}

DEFINE_TEST(DSUB)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DSUB,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == -333.0);
}

DEFINE_TEST(DMUL)
{
    Program program = CreateProgram(
        OpCode::PUSH, 123.0,
        OpCode::PUSH, 456.0,
        OpCode::DMUL,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 56088.0);
}

DEFINE_TEST(DDIV)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100.0,
        OpCode::PUSH, 20.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 5.0);
}

DEFINE_TEST(DDIV_DIV_BY_ZERO)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100.0,
        OpCode::PUSH, 0.0,
        OpCode::DDIV,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::DIV_BY_ZERO);
}

DEFINE_TEST(EQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100.0,
        OpCode::PUSH, 100.0,
        OpCode::EQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_uint == 1ull);
}

DEFINE_TEST(NEQ)
{
    Program program = CreateProgram(
        OpCode::PUSH, 100ll,
        OpCode::PUSH, 100ll,
        OpCode::NEQ,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_uint == 0ull);
}

DEFINE_TEST(JUMP)
{
    Program program = CreateProgram(
        OpCode::PUSH, (int64_t)100,
        OpCode::JUMP, (int64_t)28,
        OpCode::PUSH, (int64_t)200,
        OpCode::SYSCALL, SysCallCode::EXIT,
        OpCode::PUSH, (int64_t)300,
        OpCode::JUMP, (int64_t)27);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 300);
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
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 300);
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
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 300);
}

DEFINE_TEST(SYSCALL_EXIT)
{
    Program program = CreateProgram(
        OpCode::PUSH, (int64_t)100,
        OpCode::SYSCALL, SysCallCode::EXIT);

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 100);
}

int main()
{
    Instructions::Init();
    RUN_TEST_SUITE();
    return 0;
}