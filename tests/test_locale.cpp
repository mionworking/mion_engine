#include "test_common.hpp"

#include <fstream>
#include <string>

#include "core/locale.hpp"

static void test_locale_get_fallback_key_when_missing() {
    mion::g_locale.clear();
    EXPECT_EQ(std::string(mion::L("missing_key")), std::string("missing_key"));
}

static void test_locale_load_from_ini_file() {
    const std::string path = "/tmp/mion_test_locale.ini";
    {
        std::ofstream f(path);
        f << "[strings]\n";
        f << "menu_play=PLAY TEST\n";
        f << "ui_victory=VICTORY TEST\n";
    }

    mion::g_locale.load(path);
    EXPECT_EQ(std::string(mion::L("menu_play")), std::string("PLAY TEST"));
    EXPECT_EQ(std::string(mion::L("ui_victory")), std::string("VICTORY TEST"));
    EXPECT_EQ(std::string(mion::L("unknown")), std::string("unknown"));

    std::remove(path.c_str());
}

static void test_locale_clear_reverts_to_key() {
    mion::g_locale.strings["menu_play"] = "PLAY";
    EXPECT_EQ(std::string(mion::L("menu_play")), std::string("PLAY"));
    mion::g_locale.clear();
    EXPECT_EQ(std::string(mion::L("menu_play")), std::string("menu_play"));
}

void run_locale_tests() {
    run("Locale.FallbackKey", test_locale_get_fallback_key_when_missing);
    run("Locale.LoadFile", test_locale_load_from_ini_file);
    run("Locale.Clear", test_locale_clear_reverts_to_key);
}
