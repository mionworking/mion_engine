#pragma once
#include <cstdio>
#include <string>
#include <SDL3/SDL.h>

namespace mion {

// Lista mínima de assets críticos para boot (SFX + música + props referenciados).
inline void log_missing_assets_optional() {
    static const char* kPaths[] = {
        "assets/audio/hit.wav",
        "assets/audio/player_attack.wav",
        "assets/audio/enemy_death.wav",
        "assets/audio/player_death.wav",
        "assets/audio/parry.wav",
        "assets/audio/ui_confirm.wav",
        "assets/audio/ui_delete.wav",
        "assets/audio/dungeon_ambient.wav",
        "assets/audio/music_town.wav",
        "assets/audio/music_dungeon_calm.wav",
        "assets/audio/music_dungeon_combat.wav",
        "assets/audio/music_boss.wav",
        "assets/audio/music_victory.wav",
        "assets/audio/ambient_dungeon_drips.wav",
        "assets/audio/ambient_dungeon_torch.wav",
        "assets/audio/ambient_town_wind.wav",
        "assets/audio/ambient_town_birds.wav",
        "assets/audio/sfx_footstep_player.wav",
        "assets/audio/sfx_footstep_enemy.wav",
        "assets/audio/sfx_spell_frost.wav",
        "assets/audio/sfx_spell_chain.wav",
        "assets/audio/sfx_skill_cleave.wav",
        "assets/audio/sfx_skill_multishot.wav",
        "assets/audio/sfx_skill_poison_arrow.wav",
        "assets/audio/sfx_skill_battle_cry.wav",
        "assets/props/dungeon_pillar.png",
        "assets/props/dungeon_wall_mid.png",
    };

    for (const char* rel : kPaths) {
        std::string path;
        const char* base = SDL_GetBasePath();
        if (base) {
            path = std::string(base) + rel;
            if (FILE* f = std::fopen(path.c_str(), "rb")) {
                std::fclose(f);
                continue;
            }
        }
        path = rel;
        if (FILE* f = std::fopen(path.c_str(), "rb")) {
            std::fclose(f);
            continue;
        }
        SDL_Log("AssetManifest: em falta ou ilegivel (tentado base+cwd): %s", rel);
    }
}

} // namespace mion
