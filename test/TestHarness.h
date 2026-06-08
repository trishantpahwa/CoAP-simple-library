// Tiny shared assertion harness for the coap-simple test suites.
// Counters are defined once in test_main.cpp; every test source includes this.
#ifndef __TEST_HARNESS_H__
#define __TEST_HARNESS_H__

#include <cstdio>

extern int g_checks;
extern int g_failures;
extern const char *g_current_test;

#define CHECK(cond)                                                       \
    do                                                                    \
    {                                                                     \
        g_checks++;                                                       \
        if (!(cond))                                                      \
        {                                                                 \
            g_failures++;                                                 \
            printf("  FAIL [%s] %s:%d: %s\n", g_current_test, __FILE__,   \
                   __LINE__, #cond);                                      \
        }                                                                 \
    } while (0)

// Generic equality check (works for integers, std::string, std::vector, ...).
#define CHECK_EQ(a, b)                                                       \
    do                                                                       \
    {                                                                        \
        g_checks++;                                                          \
        if (!((a) == (b)))                                                   \
        {                                                                    \
            g_failures++;                                                    \
            printf("  FAIL [%s] %s:%d: %s == %s\n", g_current_test,          \
                   __FILE__, __LINE__, #a, #b);                              \
        }                                                                    \
    } while (0)

#define RUN(fn)                \
    do                         \
    {                          \
        g_current_test = #fn;  \
        printf("- %s\n", #fn); \
        fn();                  \
    } while (0)

#endif
