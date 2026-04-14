#include "../test_common.hpp"
#include <cstdio>
#include <string>
#include "core/config_loader.hpp"
#include "core/ini_loader.hpp"

static void test_config_defaults_when_missing() {
    mion::ConfigData cfg = mion::load_config("__no_config__.ini");
    EXPECT_EQ(cfg.window_width,  1280);
    EXPECT_EQ(cfg.window_height, 720);
    EXPECT_NEAR(cfg.volume_master, 1.0f, 0.001f);
    EXPECT_FALSE(cfg.mute);
    EXPECT_FALSE(cfg.show_autosave_indicator);
}
REGISTER_TEST(test_config_defaults_when_missing);

static void test_config_load_window_size() {
    const char* path = "mion_config_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[window]\nwidth=960\nheight=540\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_EQ(cfg.window_width,  960);
    EXPECT_EQ(cfg.window_height, 540);
    std::remove(path);
}
REGISTER_TEST(test_config_load_window_size);

static void test_config_load_autosave_indicator() {
    const char* path = "mion_config_ui_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[ui]\nautosave_indicator=1\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_TRUE(cfg.show_autosave_indicator);
    std::remove(path);
}
REGISTER_TEST(test_config_load_autosave_indicator);

static void test_config_load_mute() {
    const char* path = "mion_config_mute_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[audio]\nmute=1\nvolume_master=0.5\n", fp);
        std::fclose(fp);
    }
    mion::ConfigData cfg = mion::load_config(path);
    EXPECT_TRUE(cfg.mute);
    EXPECT_NEAR(cfg.volume_master, 0.5f, 0.001f);
    std::remove(path);
}
REGISTER_TEST(test_config_load_mute);

static void test_config_resolve_prefers_existing_base_file() {
    const char* preferred_name = "mion_config_base_path_test.ini";
    const char* fallback_path  = "mion_config_base_fallback_test.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    {
        std::FILE* fp = std::fopen(preferred_name, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("[window]\nwidth=800\n", fp);
        std::fclose(fp);
    }

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_TRUE(resolved.find(preferred_name) != std::string::npos);
    EXPECT_TRUE(resolved != fallback_path);

    std::remove(preferred_name);
    std::remove(fallback_path);
}
REGISTER_TEST(test_config_resolve_prefers_existing_base_file);

static void test_config_resolve_falls_back_to_cwd_file() {
    const char* preferred_name = "mion_config_missing_base_test.ini";
    const char* fallback_path  = "mion_config_cwd_fallback_test.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    {
        std::FILE* fp = std::fopen(fallback_path, "w");
        EXPECT_TRUE(fp != nullptr);
        std::fputs("[audio]\nmute=1\n", fp);
        std::fclose(fp);
    }

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_EQ(resolved, std::string(fallback_path));

    std::remove(preferred_name);
    std::remove(fallback_path);
}
REGISTER_TEST(test_config_resolve_falls_back_to_cwd_file);

static void test_config_resolve_returns_non_empty_when_missing() {
    const char* preferred_name = "mion_config_missing_both_test.ini";
    const char* fallback_path  = "mion_config_missing_both_fallback.ini";
    std::remove(preferred_name);
    std::remove(fallback_path);

    std::string resolved = mion::resolve_config_path(".", fallback_path, preferred_name);
    EXPECT_TRUE(!resolved.empty());
    EXPECT_TRUE(resolved.find(preferred_name) != std::string::npos);
}
REGISTER_TEST(test_config_resolve_returns_non_empty_when_missing);

static void test_config_load_ui_language_default_and_value() {
    EXPECT_EQ(mion::load_ui_language("__no_config_lang__.ini"), std::string("en"));

    const char* path = "mion_config_lang_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[ui]\nlanguage=ptbr\n", fp);
        std::fclose(fp);
    }
    EXPECT_EQ(mion::load_ui_language(path), std::string("ptbr"));
    std::remove(path);
}
REGISTER_TEST(test_config_load_ui_language_default_and_value);

static void test_config_save_runtime_config_persists_and_keeps_keybinds() {
    const char* path = "mion_runtime_config_save_test.ini";
    {
        std::FILE* fp = std::fopen(path, "w");
        std::fputs("[keybinds]\nattack=J\nconfirm=Return\n", fp);
        std::fclose(fp);
    }

    mion::ConfigData cfg{};
    cfg.window_width             = 1024;
    cfg.window_height            = 576;
    cfg.volume_master            = 0.7f;
    cfg.mute                     = true;
    cfg.show_autosave_indicator  = true;
    EXPECT_TRUE(mion::save_runtime_config(cfg, "ptbr", path));

    mion::IniData d = mion::ini_load(path);
    EXPECT_EQ(d.get_int("window", "width", 0), 1024);
    EXPECT_EQ(d.get_int("window", "height", 0), 576);
    EXPECT_NEAR(d.get_float("audio", "volume_master", 0.0f), 0.7f, 0.001f);
    EXPECT_EQ(d.get_int("audio", "mute", 0), 1);
    EXPECT_EQ(d.get_int("ui", "autosave_indicator", 0), 1);
    EXPECT_EQ(d.get_string("ui", "language", "en"), std::string("ptbr"));
    EXPECT_EQ(d.get_string("keybinds", "attack", ""), std::string("J"));
    EXPECT_EQ(d.get_string("keybinds", "confirm", ""), std::string("Return"));

    std::remove(path);
}
REGISTER_TEST(test_config_save_runtime_config_persists_and_keeps_keybinds);
