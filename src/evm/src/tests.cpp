#include "tests.h"
#include "evm.h"

INIT_TEST_SUITE();

DEFINE_TEST(PUSH)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 123);
}

DEFINE_TEST(POP)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)456,
        Instructions::OpCode::PUSH, (int64_t)789,
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::POP,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 789);
}

DEFINE_TEST(IADD)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::PUSH, (int64_t)456,
        Instructions::OpCode::IADD,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 579);
}

DEFINE_TEST(ISUB)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::PUSH, (int64_t)456,
        Instructions::OpCode::ISUB,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == -333);
}

DEFINE_TEST(IMUL)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)123,
        Instructions::OpCode::PUSH, (int64_t)456,
        Instructions::OpCode::IMUL,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 56088);
}

DEFINE_TEST(IDIV)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)100,
        Instructions::OpCode::PUSH, (int64_t)20,
        Instructions::OpCode::IDIV,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 5);
}

DEFINE_TEST(IDIV_DIV_BY_ZERO)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)100,
        Instructions::OpCode::PUSH, (int64_t)0,
        Instructions::OpCode::IDIV,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::DIV_BY_ZERO);
}

DEFINE_TEST(DADD)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, 123.0,
        Instructions::OpCode::PUSH, 456.0,
        Instructions::OpCode::DADD,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 579.0);
}

DEFINE_TEST(DSUB)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, 123.0,
        Instructions::OpCode::PUSH, 456.0,
        Instructions::OpCode::DSUB,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == -333.0);
}

DEFINE_TEST(DMUL)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, 123.0,
        Instructions::OpCode::PUSH, 456.0,
        Instructions::OpCode::DMUL,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 56088.0);
}

DEFINE_TEST(DDIV)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, 100.0,
        Instructions::OpCode::PUSH, 20.0,
        Instructions::OpCode::DDIV,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(Word(vm.GetExitCode()).as_double == 5.0);
}

DEFINE_TEST(DDIV_DIV_BY_ZERO)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, 100.0,
        Instructions::OpCode::PUSH, 0.0,
        Instructions::OpCode::DDIV,
        Instructions::OpCode::EXIT
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::DIV_BY_ZERO);
}

DEFINE_TEST(JUMP)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)100,
        Instructions::OpCode::JUMP, (int64_t)28,
        Instructions::OpCode::PUSH, (int64_t)200,
        Instructions::OpCode::EXIT,
        Instructions::OpCode::PUSH, (int64_t)300,
        Instructions::OpCode::JUMP, (int64_t)27
    );

    VM vm(std::move(program));
    VMResult result = vm.Run(64);

    ASSERT(result == VMResult::SUCCESS);
    ASSERT(vm.GetExitCode() == 300);
}

DEFINE_TEST(EXIT)
{
    Program program = Instructions::CreateProgram(
        Instructions::OpCode::PUSH, (int64_t)100,
        Instructions::OpCode::EXIT
    );

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