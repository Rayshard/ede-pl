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

        std::cout << "Running " << INSTANCES.size() << " tests..." << std::endl;

        for (size_t i = 0; i < INSTANCES.size(); i++)
        {
            auto test = INSTANCES[i];

            try
            {
                std::cout << "\t(" << i + 1 << ") ";
                test->Run();
                std::cout << "SUCCEEDED " << test->GetName() << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cout << "FAILED    " << test->GetName() << "\t" << e.what() << std::endl;
                numFailed++;
            }
        }

        if (numFailed == 0)
            std::cout << "All tests have passed!" << std::endl;
        else
            std::cout << "\nFailed " << numFailed << " tests!" << std::endl;
    }

    static void RunInstance(std::string _name)
    {

        for (size_t i = 0; i < INSTANCES.size(); i++)
        {
            auto test = INSTANCES[i];
            if (test->GetName() == _name)
            {
                try
                {
                    std::cout << "Running " << test->GetName() << "..." << std::endl;
                    test->Run();
                    std::cout << "SUCCEEDED " << test->GetName() << std::endl;
                }
                catch (const std::exception &e)
                {
                    std::cout << "FAILED    " << test->GetName() << "\t" << e.what() << std::endl;
                }

                return;
            }
        }

        std::cout << "No test found with name: " << _name << std::endl;
    }
};

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)

#define ASSERT_MSG(a, msg)                                               \
    do                                                                   \
    {                                                                    \
        if (!(a))                                                        \
        {                                                                \
            throw std::runtime_error(std::string(msg) + " " + LOCATION); \
        }                                                                \
    } while (false)

#define ASSERT(a)                               \
    do                                          \
    {                                           \
        if (!(a))                               \
        {                                       \
            throw std::runtime_error(LOCATION); \
        }                                       \
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
#define RUN_TEST(NAME) Test::RunInstance(NAME)
