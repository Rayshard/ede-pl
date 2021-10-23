#pragma once
#include <iostream>
#include <vector>

class Test
{
private:
    static std::vector<Test *> INSTANCES;

    virtual void Run() = 0;
    virtual const char *GetName() = 0;

protected:
    Test()
    {
        INSTANCES.push_back(this);
    }

public:
    static void RunInstances()
    {
        int numFailed = 0;

        std::cout << "Running " << INSTANCES.size() << " tests...\n" << std::endl;

        for (auto test : INSTANCES)
        {
            try
            {
                test->Run();
            }
            catch (const std::exception &e)
            {
                std::cout << "\tFAILED " << test->GetName() << ": " << e.what() << std::endl;
                numFailed++;
            }
        }

        if (numFailed == 0)
            std::cout << "All tests have passed!" << std::endl;
        else
            std::cout << "\nFailed " << numFailed << " tests!" << std::endl;
    }
};

#define ASSERT(a, msg)                     \
    do                                     \
    {                                      \
        if (!a)                            \
        {                                  \
            throw std::runtime_error(msg); \
        }                                  \
    } while (false)

#define DEFINE_TEST(TEST_NAME)                                 \
    class TEST_NAME : public Test                              \
    {                                                          \
        TEST_NAME() {}                                         \
        void Run() override;                                   \
                                                               \
    public:                                                    \
        TEST_NAME(TEST_NAME const &) = delete;                 \
        void operator=(TEST_NAME const &) = delete;            \
                                                               \
        const char *GetName() override { return #TEST_NAME; }  \
                                                               \
        static TEST_NAME *GetInstance()                        \
        {                                                      \
            static TEST_NAME instance;                         \
            return &instance;                                  \
        }                                                      \
                                                               \
    private:                                                   \
        static TEST_NAME *instance;                            \
    };                                                         \
                                                               \
    TEST_NAME *TEST_NAME::instance = TEST_NAME::GetInstance(); \
    void TEST_NAME::Run()

#define INIT_TEST_SUITE() std::vector<Test *> Test::INSTANCES = std::vector<Test *>()
#define RUN_TEST_SUITE() Test::RunInstances()