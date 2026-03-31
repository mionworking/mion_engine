#pragma once

#include <cstdio>
#include <cmath>
#include <cstring>

inline int s_pass = 0;
inline int s_fail = 0;

#define EXPECT_TRUE(expr)                                                                          \
    do {                                                                                           \
        if (expr) {                                                                                \
            ++s_pass;                                                                              \
        } else {                                                                                   \
            ++s_fail;                                                                              \
            std::printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #expr);                        \
        }                                                                                          \
    } while (0)

#define EXPECT_EQ(a, b) EXPECT_TRUE((a) == (b))
#define EXPECT_NE(a, b) EXPECT_TRUE((a) != (b))
#define EXPECT_GT(a, b) EXPECT_TRUE((a) > (b))
#define EXPECT_LT(a, b) EXPECT_TRUE((a) < (b))
#define EXPECT_NEAR(a, b, eps) EXPECT_TRUE(std::fabs((float)(a) - (float)(b)) < (eps))
#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define run(name, fn)                                                                              \
    do {                                                                                           \
        std::printf("[ %s ]\n", name);                                                             \
        fn();                                                                                      \
    } while (0)
