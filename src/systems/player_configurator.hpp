#pragma once
#include "../entities/actor.hpp"
#include "../components/player_config.hpp"
#include "../components/attributes.hpp"
#include "../components/spell_book.hpp"
#include "../core/sprite.hpp"
#include <optional>

namespace mion {

// ---------------------------------------------------------------
// Unified player configuration — replaces duplicated code in
// DungeonScene::_configure_player_state() and
// TownScene::_configure_player_base().
//
// Single source of truth for initializing the player Actor from
// g_player_config + derived stats. Both scenes call this instead
// of maintaining their own copies.
// ---------------------------------------------------------------

struct PlayerConfigureOptions {
    bool  reset_health_to_max   = true;
    bool  stress_mode           = false;
    float spawn_x               = 0.0f;
    float spawn_y               = 0.0f;
};

inline void configure_player(Actor& player, TextureCache* tex_cache,
                              const PlayerConfigureOptions& opts = {})
{
    player.name            = "player";
    player.team            = Team::Player;
    player.attack_damage   = g_player_config.melee_damage;
    player.ranged_damage   = g_player_config.ranged_damage;
    player.melee_hit_box   = { 22.0f, 14.0f, 28.0f };
    player.is_alive        = true;
    player.was_alive       = true;
    player.move_speed      = g_player_config.base_move_speed;
    player.collision       = { 16.0f, 16.0f };

    // Dash
    player.dash_speed               = g_player_config.dash_speed;
    player.dash_duration_seconds     = g_player_config.dash_duration;
    player.dash_cooldown_seconds     = g_player_config.dash_cooldown;
    player.dash_iframes_seconds      = g_player_config.dash_iframes;
    player.dash_active_remaining_seconds   = 0.0f;
    player.dash_cooldown_remaining_seconds = 0.0f;

    // Ranged
    player.ranged_cooldown_seconds           = g_player_config.ranged_cooldown;
    player.ranged_cooldown_remaining_seconds = 0.0f;

    // Knockback
    player.knockback_vx = 0.0f;
    player.knockback_vy = 0.0f;

    // Spell book — sync from talents
    player.spell_book = SpellBookState{};
    player.spell_book.sync_from_talents(player.talents);

    // Combat state
    player.combat.reset_for_spawn();

    // Recalculate all derived stats (damage, hp, stamina, mana max)
    recompute_player_derived_stats(
        player.derived,
        player.attributes,
        player.progression,
        player.talents,
        player.equipment,
        g_player_config.melee_damage,
        g_player_config.ranged_damage);

    // HP
    const int base_hp = opts.stress_mode ? 10000 : g_player_config.base_hp;
    player.health.max_hp = base_hp + player.derived.hp_max_bonus;
    if (opts.reset_health_to_max)
        player.health.current_hp = player.health.max_hp;

    // Stamina — always reset to base + derived bonus
    player.stamina                = StaminaState{};
    player.stamina.current        = g_player_config.base_stamina;
    player.stamina.max            = g_player_config.base_stamina_max + player.derived.stamina_max_bonus;
    player.stamina.regen_rate     = g_player_config.stamina_regen;
    player.stamina.regen_delay    = g_player_config.stamina_delay;

    // Mana — always reset to base + derived bonus
    player.mana                   = ManaState{};
    player.mana.current           = g_player_config.base_mana;
    player.mana.max               = g_player_config.base_mana_max + player.derived.mana_max_bonus;
    player.mana.regen_rate        = g_player_config.base_mana_regen;
    player.mana.regen_delay       = g_player_config.mana_regen_delay;

    // Sprite
    {
        static const char* kPlayerSheet = "assets/sprites/player.png";
        player.sprite_sheet = tex_cache
                                  ? static_cast<void*>(tex_cache->load(kPlayerSheet))
                                  : nullptr;
        if (player.sprite_sheet)
            player.anim.build_puny_clips(0, 8.0f);
    }

    // Footstep anchors
    player.footstep_prev_x     = player.transform.x;
    player.footstep_prev_y     = player.transform.y;
    player.footstep_accum_dist = 0.0f;
}

} // namespace mion
