#include "test_common.hpp"

#include <SDL3/SDL.h>

#include "core/ini_loader.hpp"
#include "core/keybind_config.hpp"

static void test_keybind_defaults_no_section() {
    mion::IniData       empty;
    mion::KeybindConfig kb = mion::load_keybinds(empty);
    EXPECT_EQ(kb.attack, SDL_SCANCODE_Z);
    EXPECT_EQ(kb.attack_alt, SDL_SCANCODE_SPACE);
    EXPECT_EQ(kb.confirm, SDL_SCANCODE_RETURN);
    EXPECT_EQ(kb.pause, SDL_SCANCODE_ESCAPE);
    EXPECT_EQ(kb.erase_save, SDL_SCANCODE_N);
    EXPECT_EQ(kb.talent_3, SDL_SCANCODE_6);
}

static void test_keybind_scancode_from_name_known() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }
    EXPECT_EQ(mion::scancode_from_name("J"), SDL_SCANCODE_J);
    EXPECT_EQ(mion::scancode_from_name("left shift"), SDL_SCANCODE_LSHIFT);
    EXPECT_EQ(mion::scancode_from_name(""), SDL_SCANCODE_UNKNOWN);
    SDL_Quit();
}

static void test_keybind_scancode_from_name_invalid_nonempty() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }
    EXPECT_EQ(mion::scancode_from_name("___not_a_real_key___"), SDL_SCANCODE_UNKNOWN);
    SDL_Quit();
}

static void test_keybind_custom_attack_from_ini() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }
    mion::IniData d;
    d.sections["keybinds"]["attack"] = "J";
    mion::KeybindConfig kb = mion::load_keybinds(d);
    EXPECT_EQ(kb.attack, SDL_SCANCODE_J);
    EXPECT_EQ(kb.attack_alt, SDL_SCANCODE_SPACE);
    SDL_Quit();
}

static void test_keybind_invalid_name_keeps_default() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }
    mion::IniData d;
    d.sections["keybinds"]["attack"] = "___not_a_real_key___";
    mion::KeybindConfig kb = mion::load_keybinds(d);
    EXPECT_EQ(kb.attack, SDL_SCANCODE_Z);
    SDL_Quit();
}

static void test_keybind_load_all_fields_from_ini() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        ++s_fail;
        std::printf("  FAIL  %s:%d  SDL_Init\n", __FILE__, __LINE__);
        return;
    }
    mion::IniData d;
    auto& sec = d.sections["keybinds"];
    sec["attack"] = "J";
    sec["attack_alt"] = "K";
    sec["ranged"] = "L";
    sec["dash"] = "Left Ctrl";
    sec["parry"] = "V";
    sec["spell_1"] = "U";
    sec["spell_2"] = "I";
    sec["spell_3"] = "O";
    sec["spell_4"] = "P";
    sec["confirm"] = "Return";
    sec["pause"] = "Backspace";
    sec["skill_tree"] = "Tab";
    sec["erase_save"] = "M";
    sec["upgrade_1"] = "7";
    sec["upgrade_2"] = "8";
    sec["upgrade_3"] = "9";
    sec["talent_1"] = "F1";
    sec["talent_2"] = "F2";
    sec["talent_3"] = "F3";

    mion::KeybindConfig kb = mion::load_keybinds(d);
    EXPECT_EQ(kb.attack, SDL_SCANCODE_J);
    EXPECT_EQ(kb.attack_alt, SDL_SCANCODE_K);
    EXPECT_EQ(kb.ranged, SDL_SCANCODE_L);
    EXPECT_EQ(kb.dash, SDL_SCANCODE_LCTRL);
    EXPECT_EQ(kb.parry, SDL_SCANCODE_V);
    EXPECT_EQ(kb.spell_1, SDL_SCANCODE_U);
    EXPECT_EQ(kb.spell_2, SDL_SCANCODE_I);
    EXPECT_EQ(kb.spell_3, SDL_SCANCODE_O);
    EXPECT_EQ(kb.spell_4, SDL_SCANCODE_P);
    EXPECT_EQ(kb.confirm, SDL_SCANCODE_RETURN);
    EXPECT_EQ(kb.pause, SDL_SCANCODE_BACKSPACE);
    EXPECT_EQ(kb.skill_tree, SDL_SCANCODE_TAB);
    EXPECT_EQ(kb.erase_save, SDL_SCANCODE_M);
    EXPECT_EQ(kb.upgrade_1, SDL_SCANCODE_7);
    EXPECT_EQ(kb.upgrade_2, SDL_SCANCODE_8);
    EXPECT_EQ(kb.upgrade_3, SDL_SCANCODE_9);
    EXPECT_EQ(kb.talent_1, SDL_SCANCODE_F1);
    EXPECT_EQ(kb.talent_2, SDL_SCANCODE_F2);
    EXPECT_EQ(kb.talent_3, SDL_SCANCODE_F3);
    SDL_Quit();
}

void run_keybind_config_tests() {
    run("Keybind.Defaults", test_keybind_defaults_no_section);
    run("Keybind.ScancodeName", test_keybind_scancode_from_name_known);
    run("Keybind.ScancodeInvalid", test_keybind_scancode_from_name_invalid_nonempty);
    run("Keybind.CustomIni", test_keybind_custom_attack_from_ini);
    run("Keybind.InvalidFallback", test_keybind_invalid_name_keeps_default);
    run("Keybind.LoadAllFields", test_keybind_load_all_fields_from_ini);
}
