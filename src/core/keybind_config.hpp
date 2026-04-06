#pragma once

#include <SDL3/SDL.h>
#include <string>

#include "ini_loader.hpp"

namespace mion {

// Action → scancode mapping (loaded from config.ini [keybinds]).
struct KeybindConfig {
    SDL_Scancode attack     = SDL_SCANCODE_Z;
    SDL_Scancode attack_alt = SDL_SCANCODE_SPACE;
    SDL_Scancode ranged     = SDL_SCANCODE_X;
    SDL_Scancode dash       = SDL_SCANCODE_LSHIFT;
    SDL_Scancode parry      = SDL_SCANCODE_C;
    SDL_Scancode spell_1    = SDL_SCANCODE_Q;
    SDL_Scancode spell_2    = SDL_SCANCODE_E;
    SDL_Scancode spell_3    = SDL_SCANCODE_R;
    SDL_Scancode spell_4    = SDL_SCANCODE_F;
    SDL_Scancode confirm    = SDL_SCANCODE_RETURN;
    SDL_Scancode pause      = SDL_SCANCODE_ESCAPE;
    SDL_Scancode skill_tree = SDL_SCANCODE_TAB;
    SDL_Scancode inventory  = SDL_SCANCODE_I;
    SDL_Scancode potion     = SDL_SCANCODE_H;
    SDL_Scancode erase_save = SDL_SCANCODE_N;
    SDL_Scancode upgrade_1  = SDL_SCANCODE_1;
    SDL_Scancode upgrade_2  = SDL_SCANCODE_2;
    SDL_Scancode upgrade_3  = SDL_SCANCODE_3;
    SDL_Scancode talent_1   = SDL_SCANCODE_4;
    SDL_Scancode talent_2   = SDL_SCANCODE_5;
    SDL_Scancode talent_3   = SDL_SCANCODE_6;
};

// Converts a key name (SDL_GetScancodeFromName, English, case-insensitive) to a scancode.
inline SDL_Scancode scancode_from_name(const std::string& name) {
    if (name.empty()) return SDL_SCANCODE_UNKNOWN;
    SDL_Scancode sc = SDL_GetScancodeFromName(name.c_str());
    return (sc != SDL_SCANCODE_UNKNOWN) ? sc : SDL_SCANCODE_UNKNOWN;
}

// Loads [keybinds] from IniData; missing or invalid keys keep their defaults.
inline KeybindConfig load_keybinds(const IniData& d) {
    KeybindConfig kb{};
    auto          get = [&](const std::string& key, SDL_Scancode def) -> SDL_Scancode {
        auto si = d.sections.find("keybinds");
        if (si == d.sections.end()) return def;
        auto ki = si->second.find(key);
        if (ki == si->second.end()) return def;
        SDL_Scancode sc = scancode_from_name(ki->second);
        return (sc != SDL_SCANCODE_UNKNOWN) ? sc : def;
    };
    kb.attack     = get("attack", kb.attack);
    kb.attack_alt = get("attack_alt", kb.attack_alt);
    kb.ranged     = get("ranged", kb.ranged);
    kb.dash       = get("dash", kb.dash);
    kb.parry      = get("parry", kb.parry);
    kb.spell_1    = get("spell_1", kb.spell_1);
    kb.spell_2    = get("spell_2", kb.spell_2);
    kb.spell_3    = get("spell_3", kb.spell_3);
    kb.spell_4    = get("spell_4", kb.spell_4);
    kb.confirm    = get("confirm", kb.confirm);
    kb.pause      = get("pause", kb.pause);
    kb.skill_tree = get("skill_tree", kb.skill_tree);
    kb.inventory  = get("inventory",  kb.inventory);
    kb.potion     = get("potion",     kb.potion);
    kb.erase_save = get("erase_save", kb.erase_save);
    kb.upgrade_1  = get("upgrade_1", kb.upgrade_1);
    kb.upgrade_2  = get("upgrade_2", kb.upgrade_2);
    kb.upgrade_3  = get("upgrade_3", kb.upgrade_3);
    kb.talent_1   = get("talent_1", kb.talent_1);
    kb.talent_2   = get("talent_2", kb.talent_2);
    kb.talent_3   = get("talent_3", kb.talent_3);
    return kb;
}

} // namespace mion
