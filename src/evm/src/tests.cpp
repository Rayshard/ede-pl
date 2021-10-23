#include "tests.h"

INIT_TEST_SUITE();

DEFINE_TEST(MyTest)
{
    ASSERT(false, "This is my assertion");
}

DEFINE_TEST(MyTest2)
{
    ASSERT(true, "This is my assertion");
}

DEFINE_TEST(MyTest3)
{
    ASSERT(false, "This is my assertion");
}

int main()
{
    RUN_TEST_SUITE();
    return 0;
}