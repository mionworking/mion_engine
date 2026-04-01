#pragma once

#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>

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

// Macro antiga mantida par não quebrar quem ainda não foi migrado!
#define run(name, fn)                                                                              \
    do {                                                                                           \
        std::printf("[ %s ]\n", name);                                                             \
        fn();                                                                                      \
    } while (0)

// --- O PODER MÁGICO DE AUTO-REGISTRO ---

struct TestDesc {
    const char* name;
    void (*fn)();
};

// Guardamos a lista de testes em um lugar secreto (singleton de Meyer)
inline std::vector<TestDesc>& GetTestRegistry() {
    static std::vector<TestDesc> registry;
    return registry;
}

// O carteiro que entrega seu teste para a lista
struct TestAutoRegistrar {
    TestAutoRegistrar(const char* name, void (*fn)()) {
        GetTestRegistry().push_back({name, fn});
    }
};

// Etiqueta Mágica: Coloque isso DEPOIS da função do seu teste, no arquivo .cpp
#define REGISTER_TEST(TestFunc) \
    static TestAutoRegistrar auto_reg_##TestFunc(#TestFunc, TestFunc)

// Essa função varre a lista e executa todos os testes
inline void RunAllRegisteredTests(const char* suite_name = "Core") {
    for (const auto& test : GetTestRegistry()) {
        std::printf("[ %s ]\n", test.name);
        test.fn();
    }
}
