#include "../test_common.hpp"
#include <string>
#include "core/engine_paths.hpp"

static void test_resolve_data_path_never_empty() {
    std::string p = mion::resolve_data_path("enemies.ini");
    EXPECT_TRUE(!p.empty());
}
REGISTER_TEST(test_resolve_data_path_never_empty);

static void test_resolve_data_path_ends_with_relative() {
    // O resultado deve sempre terminar com o nome do arquivo solicitado
    std::string p = mion::resolve_data_path("spells.ini");
    const std::string suffix = "spells.ini";
    EXPECT_TRUE(p.size() >= suffix.size());
    EXPECT_TRUE(p.compare(p.size() - suffix.size(), suffix.size(), suffix) == 0);
}
REGISTER_TEST(test_resolve_data_path_ends_with_relative);

static void test_resolve_data_path_missing_file_fallback() {
    // Arquivo inexistente: deve retornar fallback "data/xxx", nunca vazio
    std::string p = mion::resolve_data_path("__nao_existe__.ini");
    EXPECT_TRUE(!p.empty());
    EXPECT_TRUE(p.find("__nao_existe__.ini") != std::string::npos);
}
REGISTER_TEST(test_resolve_data_path_missing_file_fallback);
