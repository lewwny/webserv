#ifndef TEST_HARNESS_HPP
#define TEST_HARNESS_HPP

#include <iostream>
#include <string>

static int g_failures = 0;
static int g_tests = 0;

#define ASSERT_EQ(a,b) do { ++g_tests; if (!((a) == (b))) { \
    std::cerr << __FILE__ << ":" << __LINE__ << " FAIL: " << #a << " != " << #b \
              << " (" << (a) << " vs " << (b) << ")\n"; ++g_failures; } } while(0)

#define ASSERT_TRUE(a) do { ++g_tests; if (!(a)) { \
    std::cerr << __FILE__ << ":" << __LINE__ << " FAIL: " << #a << "\n"; ++g_failures; } } while(0)

#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

#define RUN_TEST(fn) do { \
    int before_failures = g_failures; \
    std::cout << COLOR_BLUE << "[ RUN      ] " << #fn << COLOR_RESET << std::endl; \
    fn(); \
    if (g_failures == before_failures) \
        std::cout << COLOR_GREEN << "[       OK ] " << #fn << COLOR_RESET << std::endl; \
    else \
        std::cout << COLOR_RED << "[  FAILED  ] " << #fn << COLOR_RESET << std::endl; \
} while(0)

static int finish_tests()
{
    std::cout << "\n";
    if (g_failures == 0)
        std::cout << COLOR_GREEN << "ALL TESTS PASSED (" << g_tests << ")" << COLOR_RESET << std::endl;
    else
        std::cout << COLOR_RED << "TESTS FAILED: " << g_failures << " failures out of " << g_tests << " tests" << COLOR_RESET << std::endl;
    return g_failures == 0 ? 0 : 1;
}

#endif
