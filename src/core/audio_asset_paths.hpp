#pragma once
#include <array>
#include "audio.hpp"

namespace mion::audio_assets {

inline constexpr std::array<const char*, (int)MusicState::MUSIC_STATE_COUNT> kMusicPaths = {
    nullptr,
    "assets/audio/music_town.wav",
    "assets/audio/music_dungeon_calm.wav",
    "assets/audio/music_dungeon_combat.wav",
    "assets/audio/music_boss.wav",
    "assets/audio/music_victory.wav",
};

inline constexpr std::array<const char*, (int)AmbientId::AMBIENT_COUNT> kAmbientPaths = {
    "assets/audio/ambient_dungeon_drips.wav",
    "assets/audio/ambient_dungeon_torch.wav",
    "assets/audio/ambient_town_wind.wav",
    "assets/audio/ambient_town_birds.wav",
    "assets/audio/ambient_transition_wind.wav",
};

inline constexpr std::array<const char*, (int)SoundId::COUNT> kSfxPaths = {
    "assets/audio/hit.wav",
    "assets/audio/player_attack.wav",
    "assets/audio/enemy_death.wav",
    "assets/audio/player_death.wav",
    "assets/audio/dash.wav",
    "assets/audio/ranged_attack.wav",
    "assets/audio/spell_bolt.wav",
    "assets/audio/spell_nova.wav",
    "assets/audio/sfx_spell_frost.wav",
    "assets/audio/sfx_spell_chain.wav",
    "assets/audio/sfx_skill_cleave.wav",
    "assets/audio/sfx_skill_multishot.wav",
    "assets/audio/sfx_skill_poison_arrow.wav",
    "assets/audio/sfx_skill_battle_cry.wav",
    "assets/audio/parry.wav",
    "assets/audio/ui_confirm.wav",
    "assets/audio/ui_delete.wav",
    "assets/audio/sfx_footstep_player.wav",
    "assets/audio/sfx_footstep_enemy.wav",
};

} // namespace mion::audio_assets
