// Plano máximo de testes — fases 1–9. Tilemap/lighting/font: stress manual em tools/render_stress_main.cpp.
#include "test_common.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#include "components/progression.hpp"
#include "components/player_config.hpp"
#include "components/spell_defs.hpp"
#include "components/talent_tree.hpp"
#include "components/spell_book.hpp"
#include "components/stamina.hpp"
#include "components/status_effect.hpp"
#include "core/animation.hpp"
#include "core/audio.hpp"
#include "core/camera.hpp"
#include "core/input.hpp"
#include "entities/actor.hpp"
#include "entities/enemy_type.hpp"
#include "entities/projectile.hpp"
#include "systems/enemy_ai.hpp"
#include "systems/movement_system.hpp"
#include "systems/pathfinder.hpp"
#include "systems/player_action.hpp"
#include "systems/projectile_system.hpp"
#include "systems/resource_system.hpp"
#include "systems/simple_particles.hpp"
#include "systems/status_effect_system.hpp"
#include "systems/shop_system.hpp"
#include "world/dungeon_rules.hpp"
#include "world/room.hpp"
#include "world/room_loader.hpp"
#include "world/tilemap.hpp"

#include "core/ini_loader.hpp"
#include "core/ui.hpp"
#include "core/save_system.hpp"
#include "core/quest_state.hpp"
#include "core/run_stats.hpp"
#include "entities/shop.hpp"
#include "core/engine_paths.hpp"
#include "core/scene_fader.hpp"

#include "systems/floating_text.hpp"

#include "scenes/dungeon_scene.hpp"

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------
// Fase 1 — componentes
// ---------------------------------------------------------------------------
static void test_stamina_consume_clamp_and_regen_delay() {
    mion::StaminaState s;
    s.current = 50.0f;
    s.max = 100.0f;
    s.regen_rate = 40.0f;
    s.regen_delay = 0.4f;
    s.consume(80.0f);
    EXPECT_NEAR(s.current, 0.0f, 0.001f);
    s.tick(0.2f);
    EXPECT_NEAR(s.current, 0.0f, 0.001f);
    s.tick(0.25f);
    s.tick(0.05f);
    EXPECT_TRUE(s.current > 0.0f);
}

static void test_stamina_tick_caps_at_max() {
    mion::StaminaState s;
    s.current = 95.0f;
    s.max = 100.0f;
    s.regen_rate = 1000.0f;
    s.regen_delay = 0.0f;
    s.tick(1.0f);
    EXPECT_NEAR(s.current, 100.0f, 0.01f);
}

static void test_status_effect_has_and_apply_replace() {
    mion::StatusEffectState st;
    EXPECT_FALSE(st.has(mion::StatusEffectType::Poison));
    st.apply(mion::StatusEffectType::Poison, 2.0f);
    EXPECT_TRUE(st.has(mion::StatusEffectType::Poison));
    st.apply(mion::StatusEffectType::Poison, 5.0f);
    EXPECT_TRUE(st.slots[0].remaining > 4.0f);
}

static void test_status_effect_slow_multiplier_min() {
    mion::StatusEffectState st;
    st.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.7f);
    st.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.4f);
    EXPECT_NEAR(st.slow_multiplier(), 0.4f, 0.001f);
}

static void test_status_effect_clear_expired() {
    mion::StatusEffectState st;
    st.apply(mion::StatusEffectType::Stun, 0.05f);
    st.slots[0].remaining = -0.01f;
    st.clear_expired();
    EXPECT_FALSE(st.has(mion::StatusEffectType::Stun));
}

static void test_status_effect_full_slots_overwrites_slot0() {
    mion::StatusEffectState st;
    st.apply(mion::StatusEffectType::Poison, 1.0f);
    st.apply(mion::StatusEffectType::Slow, 1.0f);
    st.apply(mion::StatusEffectType::Stun, 1.0f);
    st.apply_full(mion::StatusEffectType::Poison, 9.0f, 0.1f, 1, 0.5f);
    EXPECT_TRUE(st.slots[0].type == mion::StatusEffectType::Poison);
    EXPECT_NEAR(st.slots[0].remaining, 9.0f, 0.001f);
}

static void test_progression_add_xp_multi_level() {
    mion::ProgressionState p;
    // Nível 1→2 custa 100 XP; nível 2→3 custa 200 XP (xp_to_next = level * 100).
    int g = p.add_xp(300);
    EXPECT_EQ(g, 2);
    EXPECT_EQ(p.level, 3);
    EXPECT_EQ(p.pending_level_ups, 2);
    EXPECT_TRUE(p.level_choice_pending());
}

static void test_progression_pick_upgrades_reduce_pending() {
    mion::ProgressionState p;
    p.pending_level_ups = 2;
    p.pick_upgrade_damage();
    EXPECT_EQ(p.pending_level_ups, 1);
    EXPECT_EQ(p.bonus_attack_damage, 3);
    EXPECT_NEAR(p.spell_damage_multiplier, 1.08f, 0.001f);
    p.pick_upgrade_hp();
    EXPECT_EQ(p.pending_level_ups, 0);
    EXPECT_EQ(p.bonus_max_hp, 15);
}

static void test_ini_get_string() {
    mion::IniData d;
    d.sections["sec"]["k"] = "hello";
    EXPECT_TRUE(d.get_string("sec", "k", "") == "hello");
    EXPECT_TRUE(d.get_string("sec", "missing", "x") == "x");
}

static void test_progression_ini_xp_curve() {
    mion::ProgressionConfig saved = mion::g_progression_config;
    mion::reset_progression_config_defaults();
    mion::IniData d;
    d.sections["xp"]["xp_base"]        = "40";
    d.sections["xp"]["xp_level_scale"] = "25";
    mion::apply_progression_ini(d);
    mion::ProgressionState p{};
    EXPECT_EQ(p.xp_to_next, 40);
    (void)p.add_xp(40);
    EXPECT_EQ(p.level, 2);
    EXPECT_EQ(p.xp_to_next, 65);
    mion::g_progression_config = saved;
}

static void test_player_ini_applies_fields() {
    mion::PlayerConfig saved = mion::g_player_config;
    mion::reset_player_config_defaults();
    mion::IniData d;
    d.sections["player"]["melee_damage"]   = "17";
    d.sections["player"]["ranged_damage"]  = "11";
    d.sections["player"]["base_move_speed"] = "155.5";
    mion::apply_player_ini(d);
    EXPECT_EQ(mion::g_player_config.melee_damage, 17);
    EXPECT_EQ(mion::g_player_config.ranged_damage, 11);
    EXPECT_NEAR(mion::g_player_config.base_move_speed, 155.5f, 0.001f);
    mion::g_player_config = saved;
}

static void test_talents_ini_display_name() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["display_name"] = "Fluxo de Mana";
    mion::apply_talents_ini(d);
    EXPECT_TRUE(mion::talent_display_name(mion::TalentId::ManaFlow) == "Fluxo de Mana");
    mion::reset_talent_tree_defaults();
}

static void test_spell_def_effective_scaling() {
    mion::SpellDef def{};
    def.damage              = 10;
    def.damage_per_level    = 4;
    def.cooldown_seconds    = 1.0f;
    def.cooldown_per_level  = 0.15f;
    EXPECT_EQ(def.effective_damage(2), 18);
    EXPECT_NEAR(def.effective_cooldown(2), 0.70f, 0.001f);
    EXPECT_NEAR(def.effective_cooldown(100), 0.05f, 0.001f);
}

static void test_ini_load_get_string_from_file() {
    const char* path = "mion_ini_string_roundtrip.ini";
    std::FILE*  fp   = std::fopen(path, "w");
    EXPECT_TRUE(fp != nullptr);
    std::fputs("[labels]\ntitle=Olá INI\nempty_default=x\n", fp);
    std::fclose(fp);

    mion::IniData d = mion::ini_load(path);
    EXPECT_TRUE(d.get_string("labels", "title", "") == "Olá INI");
    EXPECT_TRUE(d.get_string("labels", "missing", "fallback") == "fallback");
    std::remove(path);
}

static void test_apply_enemy_ini_section_overrides() {
    mion::IniData d;
    d.sections["skeleton"]["max_hp"]       = "99";
    d.sections["skeleton"]["sprite_scale"] = "2.5";
    d.sections["skeleton"]["dir_row"]      = "2";
    d.sections["skeleton"]["sprite_sheet_path"] = "assets/test_enemy.png";

    mion::EnemyDef def = mion::get_enemy_def(mion::EnemyType::Skeleton);
    const int      orig_hp = def.max_hp;
    std::string    storage;
    mion::apply_enemy_ini_section(d, "skeleton", def, &storage);

    EXPECT_EQ(def.max_hp, 99);
    EXPECT_NEAR(def.sprite_scale, 2.5f, 0.001f);
    EXPECT_EQ(def.dir_row, 2);
    EXPECT_TRUE(std::strcmp(def.sprite_sheet_path, storage.c_str()) == 0);
    EXPECT_TRUE(std::strstr(def.sprite_sheet_path, "test_enemy.png") != nullptr);

    mion::IniData d2;
    mion::EnemyDef def2 = mion::get_enemy_def(mion::EnemyType::Skeleton);
    mion::apply_enemy_ini_section(d2, "skeleton", def2, nullptr);
    EXPECT_EQ(def2.max_hp, orig_hp);
}

static void test_apply_spell_ini_section_per_level_fields() {
    mion::IniData d;
    d.sections["bolt"]["damage"]            = "12";
    d.sections["bolt"]["damage_per_level"]  = "5";
    d.sections["bolt"]["cooldown_per_level"] = "0.05";

    mion::SpellDef def = mion::g_spell_defs[0];
    def.id             = mion::SpellId::FrostBolt;
    def.damage         = 16;
    def.cooldown_seconds = 0.5f;
    def.damage_per_level   = 0;
    def.cooldown_per_level = 0.0f;

    mion::apply_spell_ini_section(d, "bolt", def);
    EXPECT_EQ(def.damage, 12);
    EXPECT_EQ(def.damage_per_level, 5);
    EXPECT_NEAR(def.cooldown_per_level, 0.05f, 0.001f);
    EXPECT_EQ(def.effective_damage(2), 12 + 10);
    EXPECT_NEAR(def.effective_cooldown(4), 0.30f, 0.001f);
}

static void test_progression_ini_custom_level_up_bonuses() {
    mion::ProgressionConfig saved = mion::g_progression_config;
    mion::reset_progression_config_defaults();
    mion::IniData d;
    d.sections["level_up"]["damage_bonus"]     = "7";
    d.sections["level_up"]["hp_bonus"]         = "22";
    d.sections["level_up"]["speed_bonus"]        = "9.5";
    d.sections["level_up"]["spell_mult_bonus"] = "0";
    mion::apply_progression_ini(d);

    mion::ProgressionState p{};
    p.pending_level_ups = 1;
    p.pick_upgrade_damage();
    EXPECT_EQ(p.bonus_attack_damage, 7);
    EXPECT_NEAR(p.spell_damage_multiplier, 1.0f, 0.001f);

    p.pending_level_ups = 1;
    p.pick_upgrade_hp();
    EXPECT_EQ(p.bonus_max_hp, 22);

    p.pending_level_ups = 1;
    p.pick_upgrade_speed();
    EXPECT_NEAR(p.bonus_move_speed, 9.5f, 0.001f);

    mion::g_progression_config = saved;
}

static void test_player_ini_clamps_mana_and_stamina_max() {
    mion::PlayerConfig saved = mion::g_player_config;
    mion::reset_player_config_defaults();
    mion::IniData d;
    d.sections["player"]["base_mana"]      = "120";
    d.sections["player"]["base_mana_max"]  = "50";
    d.sections["player"]["base_stamina"]     = "80";
    d.sections["player"]["base_stamina_max"] = "40";
    mion::apply_player_ini(d);
    EXPECT_NEAR(mion::g_player_config.base_mana_max, 120.0f, 0.001f);
    EXPECT_NEAR(mion::g_player_config.base_stamina_max, 80.0f, 0.001f);
    mion::g_player_config = saved;
}

static void test_talent_ini_cost_per_level_affects_unlock() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["cost_per_level"] = "3";
    mion::apply_talents_ini(d);

    mion::TalentState t{};
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_EQ(t.pending_points, 2);
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ManaFlow));

    mion::reset_talent_tree_defaults();
}

static void test_data_repo_inis_parse_if_present() {
    const char* files[] = {
        "progression.ini",
        "player.ini",
        "talents.ini",
        "enemies.ini",
        "spells.ini",
    };
    for (const char* rel : files) {
        mion::IniData data = mion::ini_load(mion::resolve_data_path(rel));
        if (data.sections.empty())
            continue;

        if (std::strcmp(rel, "progression.ini") == 0) {
            mion::ProgressionConfig stash = mion::g_progression_config;
            mion::reset_progression_config_defaults();
            mion::apply_progression_ini(data);
            EXPECT_TRUE(mion::g_progression_config.xp_base > 0);
            mion::g_progression_config = stash;
        } else if (std::strcmp(rel, "player.ini") == 0) {
            mion::PlayerConfig stash = mion::g_player_config;
            mion::reset_player_config_defaults();
            mion::apply_player_ini(data);
            EXPECT_TRUE(mion::g_player_config.base_hp > 0);
            EXPECT_TRUE(mion::g_player_config.melee_damage > 0);
            mion::g_player_config = stash;
        } else if (std::strcmp(rel, "talents.ini") == 0) {
            mion::reset_talent_tree_defaults();
            mion::apply_talents_ini(data);
            EXPECT_TRUE(!mion::talent_display_name(mion::TalentId::ManaFlow).empty());
            mion::reset_talent_tree_defaults();
        } else if (std::strcmp(rel, "enemies.ini") == 0) {
            mion::EnemyDef def = mion::get_enemy_def(mion::EnemyType::Skeleton);
            std::string store;
            mion::apply_enemy_ini_section(data, "skeleton", def, &store);
            EXPECT_TRUE(def.max_hp > 0);
        } else if (std::strcmp(rel, "spells.ini") == 0) {
            mion::SpellDef def{};
            def.id = mion::SpellId::FrostBolt;
            mion::apply_spell_ini_section(data, "frost_bolt", def);
            EXPECT_TRUE(def.damage >= 0);
        }
    }
}

static void test_player_action_ranged_uses_actor_ranged_damage() {
    mion::PlayerActionSystem pas;
    mion::Actor             player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.stamina.current = 100.0f;
    player.facing_x        = 1.0f;
    player.facing_y        = 0.0f;
    player.transform.set_position(0.0f, 0.0f);
    player.progression.bonus_attack_damage = 4;
    mion::recompute_player_derived_stats(
        player.derived, player.attributes, player.progression,
        player.talents, player.equipment, /*base_melee=*/10, /*base_ranged=*/7);
    std::vector<mion::Projectile> prs;
    mion::InputState in;
    in.ranged_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, &prs, nullptr);
    EXPECT_EQ((int)prs.size(), 1);
    EXPECT_EQ(prs[0].damage, 9);
}

static void test_actor_effective_move_speed_player_bonus_and_slow() {
    mion::Actor a;
    a.team = mion::Team::Player;
    a.move_speed = 100.0f;
    a.progression.bonus_move_speed = 50.0f;
    EXPECT_NEAR(a.effective_move_speed(), 150.0f, 0.001f);
    a.status_effects.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.5f);
    EXPECT_NEAR(a.effective_move_speed(), 75.0f, 0.001f);
}

static void test_actor_is_dashing() {
    mion::Actor a;
    a.dash_active_remaining_seconds = 0.0f;
    EXPECT_FALSE(a.is_dashing());
    a.dash_active_remaining_seconds = 0.01f;
    EXPECT_TRUE(a.is_dashing());
}

// ---------------------------------------------------------------------------
// Fase 2 — sistemas
// ---------------------------------------------------------------------------
static void test_resource_system_ticks_mana_and_stamina() {
    mion::Actor a;
    a.is_alive = true;
    a.mana.current = 10.0f;
    a.mana.max = 100.0f;
    a.mana.regen_rate = 20.0f;
    a.mana.regen_delay = 0.0f;
    a.stamina.current = 10.0f;
    a.stamina.max = 100.0f;
    a.stamina.regen_rate = 30.0f;
    a.stamina.regen_delay = 0.0f;
    mion::ResourceSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 1.0f);
    EXPECT_NEAR(a.mana.current, 30.0f, 0.01f);
    EXPECT_NEAR(a.stamina.current, 40.0f, 0.01f);
}

static void test_resource_system_skips_dead() {
    mion::Actor a;
    a.is_alive = false;
    a.mana.current = 10.0f;
    mion::ResourceSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 1.0f);
    EXPECT_NEAR(a.mana.current, 10.0f, 0.001f);
}

static void test_movement_system_resets_moving_and_knockback() {
    mion::Actor a, b;
    a.is_alive = true;
    b.is_alive = true;
    a.is_moving = true;
    b.is_moving = true;
    a.transform.set_position(0.0f, 0.0f);
    a.apply_knockback_impulse(60.0f, 0.0f);
    mion::MovementSystem sys;
    std::vector<mion::Actor*> actors = { &a, &b };
    sys.fixed_update(actors, 1.0f / 60.0f);
    EXPECT_FALSE(a.is_moving);
    EXPECT_FALSE(b.is_moving);
    EXPECT_TRUE(std::fabs(a.transform.x) > 0.0f || std::fabs(a.knockback_vx) > 0.0f);
}

static void test_status_effect_system_poison_ticks() {
    mion::Actor a;
    a.is_alive = true;
    a.health = { 100, 100 };
    a.status_effects.apply_full(mion::StatusEffectType::Poison, 2.0f, 0.1f, 7, 0);
    mion::StatusEffectSystem sys;
    std::vector<mion::Actor*> actors = { &a };
    sys.fixed_update(actors, 0.25f);
    EXPECT_TRUE(a.health.current_hp < 100);
    EXPECT_TRUE(a.is_alive);
}

static void test_projectile_expires_on_lifetime() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 100.0f;
    p.y = 100.0f;
    p.vx = p.vy = 0.0f;
    p.lifetime_remaining_seconds = 0.02f;
    prs.push_back(p);
    mion::RoomDefinition room;
    room.bounds = { 0, 500, 0, 500 };
    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.05f);
    EXPECT_EQ((int)prs.size(), 0);
}

static void test_projectile_despawns_outside_room() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = p.y = 100.0f;
    p.vx = 5000.0f;
    p.vy = 0.0f;
    p.half_w = p.half_h = 5.0f;
    p.lifetime_remaining_seconds = 10.0f;
    prs.push_back(p);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.05f);
    EXPECT_EQ((int)prs.size(), 0);
}

static void test_scene_fader_fade_out_holds_black() {
    mion::SceneFader f;
    f.start_fade_out(0.5f);
    f.tick(0.6f);
    EXPECT_TRUE(f.is_black_hold());
    EXPECT_EQ((int)f.alpha, 255);
}

static void test_scene_fader_fade_in_goes_clear() {
    mion::SceneFader f;
    f.start_fade_out(0.1f);
    f.tick(0.2f);
    EXPECT_TRUE(f.is_black_hold());
    f.start_fade_in(0.2f);
    f.tick(0.25f);
    EXPECT_TRUE(f.is_clear());
    EXPECT_EQ((int)f.alpha, 0);
}

static void test_floating_text_tick_erases_dead() {
    mion::FloatingTextSystem sys;
    sys.spawn(0.f, 0.f, "x", {255, 255, 255, 255}, 1, 0.1f, 0.f);
    EXPECT_EQ(sys.texts.size(), (size_t)1);
    sys.tick(0.15f);
    EXPECT_EQ(sys.texts.size(), (size_t)0);
}

static void test_sfx_distance_attenuation_quadratic() {
    EXPECT_NEAR(mion::sfx_distance_attenuation(0.f, 600.f), 1.0f, 0.001f);
    EXPECT_NEAR(mion::sfx_distance_attenuation(600.f * 600.f, 600.f), 0.0f, 0.001f);
    EXPECT_NEAR(mion::sfx_distance_attenuation(300.f * 300.f, 600.f), 0.25f, 0.001f);
}

static void test_projectile_hits_enemy_and_applies_damage() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 0.0f;
    p.y = 0.0f;
    p.vx = p.vy = 0.0f;
    p.half_w = 20.0f;
    p.half_h = 20.0f;
    p.damage = 25;
    p.owner_team = mion::Team::Player;
    p.lifetime_remaining_seconds = 1.0f;
    prs.push_back(p);

    mion::Actor enemy;
    enemy.name = "target_a";
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.hurt_box = { 14.0f, 14.0f };
    enemy.health = { 100, 100 };
    enemy.combat.reset_for_spawn();

    mion::RoomDefinition room;
    room.bounds = { -500, 500, -500, 500 };
    std::vector<mion::Actor*> actors = { &enemy };
    psys.fixed_update(prs, actors, room, 0.016f);
    EXPECT_EQ(enemy.health.current_hp, 75);
    EXPECT_TRUE(psys.projectile_hit_actor);
    EXPECT_EQ((int)prs.size(), 0);
    EXPECT_EQ(psys.last_hit_events.size(), (size_t)1);
    EXPECT_TRUE(psys.last_hit_events[0].target_name == "target_a");
    EXPECT_EQ(psys.last_hit_events[0].damage, 25);
}

static void test_projectile_skips_same_team() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = p.y = 0.0f;
    p.vx = p.vy = 0.0f;
    p.half_w = 30.0f;
    p.half_h = 30.0f;
    p.damage = 50;
    p.owner_team = mion::Team::Player;
    p.lifetime_remaining_seconds = 1.0f;
    prs.push_back(p);

    mion::Actor ally;
    ally.team = mion::Team::Player;
    ally.is_alive = true;
    ally.transform.set_position(0.0f, 0.0f);
    ally.hurt_box = { 14.0f, 14.0f };
    ally.health = { 100, 100 };
    ally.combat.reset_for_spawn();

    mion::RoomDefinition room;
    room.bounds = { -500, 500, -500, 500 };
    std::vector<mion::Actor*> actors = { &ally };
    psys.fixed_update(prs, actors, room, 0.016f);
    EXPECT_EQ(ally.health.current_hp, 100);
}

static void test_projectile_blocked_by_obstacle() {
    mion::ProjectileSystem psys;
    std::vector<mion::Projectile> prs;
    mion::Projectile p;
    p.active = true;
    p.x = 50.0f;
    p.y = 100.0f;
    p.vx = 200.0f;
    p.vy = 0.0f;
    p.half_w = p.half_h = 4.0f;
    p.lifetime_remaining_seconds = 2.0f;
    prs.push_back(p);

    mion::RoomDefinition room;
    room.bounds = { 0, 400, 0, 400 };
    room.add_obstacle("x", 80, 80, 120, 120);

    std::vector<mion::Actor*> actors;
    psys.fixed_update(prs, actors, room, 0.2f);
    EXPECT_EQ((int)prs.size(), 0);
}

// ---------------------------------------------------------------------------
// Fase 3 — PlayerActionSystem
// ---------------------------------------------------------------------------
static void test_player_action_moves_and_sets_facing() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.team = mion::Team::Player;
    player.move_speed = 200.0f;
    player.transform.set_position(0.0f, 0.0f);
    mion::InputState in;
    in.move_x = 1.0f;
    in.move_y = 0.0f;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.is_moving);
    EXPECT_NEAR(player.facing_x, 1.0f, 0.01f);
    EXPECT_TRUE(player.transform.x > 0.0f);
}

static void test_player_action_melee_requires_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.stamina.current = 0.0f;
    mion::InputState in;
    in.attack_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.combat.is_attack_idle());
}

static void test_player_action_melee_begins_attack_with_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.stamina.current = 100.0f;
    mion::InputState in;
    in.attack_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_EQ(player.combat.attack_phase, mion::AttackPhase::Startup);
}

static void test_player_action_ranged_spawns_and_damage_formula() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.stamina.current = 100.0f;
    player.facing_x = 1.0f;
    player.facing_y = 0.0f;
    player.transform.set_position(0.0f, 0.0f);
    player.progression.bonus_attack_damage = 4;
    mion::recompute_player_derived_stats(
        player.derived, player.attributes, player.progression,
        player.talents, player.equipment, /*base_melee=*/10, /*base_ranged=*/8);
    std::vector<mion::Projectile> prs;
    mion::InputState in;
    in.ranged_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, &prs, nullptr);
    EXPECT_EQ((int)prs.size(), 1);
    EXPECT_EQ(prs[0].damage, 10);
}

static void test_player_action_bolt_consumes_mana() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.mana.current = 100.0f;
    player.facing_x = 1.0f;
    player.facing_y = 0.0f;
    player.spell_book.unlock(mion::SpellId::FrostBolt);
    std::vector<mion::Projectile> prs;
    mion::InputState in;
    in.spell_1_pressed = true;
    float mana_before = player.mana.current;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, &prs, nullptr);
    EXPECT_TRUE(player.mana.current < mana_before);
    EXPECT_EQ((int)prs.size(), 1);
}

static void test_player_action_nova_hits_enemy_list() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.team = mion::Team::Player;
    player.combat.reset_for_spawn();
    player.mana.current = 100.0f;
    player.spell_book.unlock(mion::SpellId::Nova);
    player.transform.set_position(0.0f, 0.0f);
    mion::Actor enemy;
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.transform.set_position(40.0f, 0.0f);
    enemy.health = { 100, 100 };
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    mion::InputState in;
    in.spell_2_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, &actors);
    EXPECT_TRUE(enemy.health.current_hp < 100);
}

static void test_player_action_dash_consumes_stamina() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    player.stamina.current = 100.0f;
    player.facing_x = 0.0f;
    player.facing_y = 1.0f;
    mion::InputState in;
    in.dash_pressed = true;
    in.move_x = 1.0f;
    in.move_y = 0.0f;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_NEAR(player.stamina.current, 72.0f, 1.0f);
    EXPECT_TRUE(player.is_dashing());
}

static void test_player_action_stun_blocks_movement() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.move_speed = 200.0f;
    player.transform.set_position(0.0f, 0.0f);
    player.status_effects.apply(mion::StatusEffectType::Stun, 1.0f);
    mion::InputState in;
    in.move_x = 1.0f;
    float x0 = player.transform.x;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_NEAR(player.transform.x, x0, 0.001f);
    EXPECT_FALSE(player.is_moving);
}

static void test_player_action_parry_sets_window() {
    mion::PlayerActionSystem pas;
    mion::Actor player;
    player.is_alive = true;
    player.combat.reset_for_spawn();
    mion::InputState in;
    in.parry_pressed = true;
    pas.fixed_update(player, in, 1.0f / 60.0f, nullptr, nullptr, nullptr);
    EXPECT_TRUE(player.combat.parry_window_remaining_seconds > 0.0f);
}

// ---------------------------------------------------------------------------
// Fase 4 — pathfinding
// ---------------------------------------------------------------------------
static void test_nav_grid_obstacle_blocks_tile() {
    mion::Tilemap tm;
    tm.init(5, 5, 32);
    tm.fill(0, 0, 4, 4, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    room.add_obstacle("b", 48, 48, 80, 80);
    mion::NavGrid grid;
    grid.build(tm, room);
    EXPECT_FALSE(grid.is_walkable(1, 1));
    EXPECT_TRUE(grid.is_walkable(0, 0));
}

static void test_nav_grid_world_to_tile_clamp() {
    mion::Tilemap tm;
    tm.init(3, 3, 32);
    tm.fill(0, 0, 2, 2, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    mion::NavGrid grid;
    grid.build(tm, room);
    int c = -1, r = -1;
    grid.world_to_tile(-100.0f, -100.0f, c, r);
    EXPECT_EQ(c, 0);
    EXPECT_EQ(r, 0);
    grid.world_to_tile(1.0e6f, 1.0e6f, c, r);
    EXPECT_EQ(c, 2);
    EXPECT_EQ(r, 2);
}

static void test_pathfinder_same_tile_valid_empty_waypoints() {
    mion::Tilemap tm;
    tm.init(4, 4, 32);
    tm.fill(0, 0, 3, 3, mion::TileType::Floor);
    mion::RoomDefinition room;
    room.bounds = { 0, 500, 0, 500 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(48.0f, 48.0f, 50.0f, 50.0f);
    EXPECT_TRUE(path.valid);
    EXPECT_TRUE(path.waypoints.empty());
}

static void test_pathfinder_find_path_around_wall() {
    mion::Tilemap tm;
    tm.init(3, 3, 32);
    tm.fill(0, 0, 2, 2, mion::TileType::Floor);
    tm.set(1, 0, mion::TileType::Wall);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 200 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(16.0f, 16.0f, 80.0f, 16.0f);
    EXPECT_TRUE(path.valid);
    EXPECT_TRUE(!path.waypoints.empty());
}

static void test_pathfinder_unreachable_goal() {
    mion::Tilemap tm;
    tm.init(3, 1, 32);
    tm.fill(0, 0, 2, 0, mion::TileType::Floor);
    tm.set(1, 0, mion::TileType::Wall);
    mion::RoomDefinition room;
    room.bounds = { 0, 200, 0, 64 };
    mion::Pathfinder pf;
    pf.build_nav(tm, room);
    mion::Path path = pf.find_path(16.0f, 16.0f, 80.0f, 16.0f);
    EXPECT_FALSE(path.valid);
}

static void test_path_advance_and_done() {
    mion::Path p;
    p.valid = true;
    p.waypoints.push_back({ 1.0f, 2.0f });
    p.waypoints.push_back({ 3.0f, 4.0f });
    p.current = 0;
    EXPECT_FALSE(p.done());
    EXPECT_NEAR(p.next_wp().x, 1.0f, 0.001f);
    p.advance();
    EXPECT_NEAR(p.next_wp().x, 3.0f, 0.001f);
    p.advance();
    EXPECT_TRUE(p.done());
    p.reset();
    EXPECT_FALSE(p.valid);
}

// ---------------------------------------------------------------------------
// Fase 5 — animação e câmara
// ---------------------------------------------------------------------------
static void test_anim_player_loop_and_non_loop_finished() {
    mion::AnimPlayer ap;
    mion::AnimClip loop_clip;
    loop_clip.loop = true;
    loop_clip.frames.push_back({ { 0, 0, 8, 8 }, 0.05f });
    loop_clip.frames.push_back({ { 8, 0, 8, 8 }, 0.05f });
    ap.clips[(int)mion::ActorAnim::Idle] = loop_clip;
    ap.play(mion::ActorAnim::Idle);
    ap.advance(0.08f);
    EXPECT_FALSE(ap.finished);
    EXPECT_TRUE(ap.current_frame() != nullptr);

    mion::AnimClip once;
    once.loop = false;
    once.frames.push_back({ { 0, 0, 4, 4 }, 0.02f });
    ap.clips[(int)mion::ActorAnim::Death] = once;
    ap.play(mion::ActorAnim::Death);
    ap.advance(0.05f);
    EXPECT_TRUE(ap.finished);
}

static void test_anim_player_play_same_clip_no_restart_until_finished() {
    mion::AnimPlayer ap;
    mion::AnimClip c;
    c.loop = false;
    c.frames.push_back({ { 0, 0, 8, 8 }, 0.2f });
    ap.clips[(int)mion::ActorAnim::Hurt] = c;
    ap.play(mion::ActorAnim::Hurt);
    ap.advance(0.05f);
    int fi = ap.frame_index;
    ap.play(mion::ActorAnim::Hurt);
    EXPECT_EQ(ap.frame_index, fi);
}

static void test_anim_player_current_frame_null_when_empty() {
    mion::AnimPlayer ap;
    EXPECT_TRUE(ap.current_frame() == nullptr);
}

static void test_anim_player_puny_dir_row_updates_y() {
    mion::AnimPlayer ap;
    ap.build_puny_clips(0, 10.0f);
    int y0 = ap.clips[(int)mion::ActorAnim::Idle].frames[0].src.y;
    ap.update_puny_dir_row(2);
    int y1 = ap.clips[(int)mion::ActorAnim::Idle].frames[0].src.y;
    EXPECT_EQ(y1 - y0, 64);
}

static void test_camera_follow_clamps() {
    mion::Camera2D cam;
    cam.viewport_w = 200.0f;
    cam.viewport_h = 150.0f;
    mion::WorldBounds b{ 0.0f, 400.0f, 0.0f, 300.0f };
    cam.follow(200.0f, 150.0f, b);
    EXPECT_NEAR(cam.x, 100.0f, 0.01f);
    EXPECT_NEAR(cam.y, 75.0f, 0.01f);
    cam.follow(0.0f, 0.0f, b);
    EXPECT_NEAR(cam.x, 0.0f, 0.01f);
    EXPECT_NEAR(cam.y, 0.0f, 0.01f);
    cam.follow(1000.0f, 1000.0f, b);
    EXPECT_NEAR(cam.x, 200.0f, 0.01f);
    EXPECT_NEAR(cam.y, 150.0f, 0.01f);
}

static void test_camera_world_to_screen_no_shake() {
    mion::Camera2D cam;
    cam.x = 50.0f;
    cam.y = 40.0f;
    EXPECT_NEAR(cam.world_to_screen_x(150.0f), 100.0f, 0.01f);
    EXPECT_NEAR(cam.world_to_screen_y(90.0f), 50.0f, 0.01f);
}

static void test_camera_shake_steps_down() {
    std::srand(12345);
    mion::Camera2D cam;
    cam.trigger_shake(10.0f, 4);
    int start = cam.shake_remaining;
    std::mt19937 rng(12345);
    for (int i = 0; i < 4; ++i)
        cam.step_shake(rng);
    EXPECT_EQ(start, 4);
    EXPECT_EQ(cam.shake_remaining, 0);
}

// ---------------------------------------------------------------------------
// Fase 6 — Enemy AI
// ---------------------------------------------------------------------------
static void test_enemy_ai_chases_player_without_pathfinder() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(300.0f, 100.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 120.0f;
    enemy.aggro_range = 500.0f;
    enemy.attack_range_ai = 40.0f;
    enemy.stop_range_ai = 25.0f;
    enemy.transform.set_position(100.0f, 100.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 30.0f, nullptr);
    EXPECT_TRUE(enemy.transform.x > 100.0f);
    EXPECT_TRUE(enemy.is_moving);
}

static void test_enemy_ai_respects_stop_range_no_move() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(10.0f, 0.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 100.0f;
    enemy.aggro_range = 500.0f;
    enemy.attack_range_ai = 50.0f;
    enemy.stop_range_ai = 30.0f;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.combat.reset_for_spawn();
    float x0 = enemy.transform.x;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr);
    EXPECT_NEAR(enemy.transform.x, x0, 2.0f);
    EXPECT_FALSE(enemy.is_moving);
}

static void test_enemy_ai_begins_attack_when_in_range() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(40.0f, 0.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.move_speed = 0.0f;
    enemy.aggro_range = 500.0f;
    enemy.attack_range_ai = 50.0f;
    enemy.stop_range_ai = 10.0f;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr);
    EXPECT_EQ(enemy.combat.attack_phase, mion::AttackPhase::Startup);
}

static void test_enemy_def_boss_grimjaw_boss_phased_and_gold() {
    const mion::EnemyDef& d = mion::get_enemy_def(mion::EnemyType::BossGrimjaw);
    EXPECT_TRUE(d.ai_behavior == mion::AiBehavior::BossPhased);
    EXPECT_TRUE(d.gold_drop_min >= 25);
    EXPECT_TRUE(d.gold_drop_max >= d.gold_drop_min);
}

static void test_parse_ai_behavior_string_variants() {
    EXPECT_TRUE(mion::parse_ai_behavior_string("melee") == mion::AiBehavior::Melee);
    EXPECT_TRUE(mion::parse_ai_behavior_string("RANGED") == mion::AiBehavior::Ranged);
    EXPECT_TRUE(mion::parse_ai_behavior_string("boss_phased") == mion::AiBehavior::BossPhased);
}

static void test_run_stats_reset_clears_fields() {
    mion::RunStats r;
    r.enemies_killed = 5;
    r.time_seconds   = 12.5f;
    r.boss_defeated  = true;
    r.reset();
    EXPECT_EQ(r.enemies_killed, 0);
    EXPECT_NEAR(r.time_seconds, 0.0f, 0.001f);
    EXPECT_FALSE(r.boss_defeated);
}

static void test_difficulty_spawn_and_multipliers() {
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(5, mion::DifficultyLevel::Easy), 4);
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(1, mion::DifficultyLevel::Easy), 1);
    EXPECT_EQ(mion::difficulty_adjust_spawn_budget(3, mion::DifficultyLevel::Hard), 4);
    float hp, dm;
    mion::difficulty_enemy_multipliers(mion::DifficultyLevel::Easy, hp, dm);
    EXPECT_NEAR(hp, 0.75f, 0.001f);
    EXPECT_NEAR(dm, 0.75f, 0.001f);
    mion::difficulty_enemy_multipliers(mion::DifficultyLevel::Hard, hp, dm);
    EXPECT_NEAR(hp, 1.35f, 0.001f);
    EXPECT_NEAR(dm, 1.25f, 0.001f);
}

static void test_save_roundtrip_meta_run_fields() {
    mion::SaveData d{};
    d.gold             = 12;
    d.victory_reached  = true;
    d.difficulty       = 2;
    d.last_run_stats.enemies_killed = 7;
    d.last_run_stats.time_seconds   = 99.5f;
    d.last_run_stats.boss_defeated  = true;
    const char* path = "mion_save_meta_run_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_TRUE(out.victory_reached);
    EXPECT_EQ(out.difficulty, 2);
    EXPECT_EQ(out.last_run_stats.enemies_killed, 7);
    EXPECT_NEAR(out.last_run_stats.time_seconds, 99.5f, 0.05f);
    EXPECT_TRUE(out.last_run_stats.boss_defeated);
    std::remove(path);
}

static void test_enemy_ai_patrol_alert_neighbor_chase() {
    mion::EnemyAISystem sys;
    mion::Actor         player, guard_a, guard_b;
    player.team     = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(100.0f, 0.0f);
    guard_a.team         = mion::Team::Enemy;
    guard_a.is_alive     = true;
    guard_a.ai_behavior    = mion::AiBehavior::Patrol;
    guard_a.aggro_range  = 280.0f;
    guard_a.move_speed   = 60.0f;
    guard_a.patrol_state = mion::PatrolState::Patrol;
    guard_a.patrol_waypoints = {{0.0f, 0.0f}, {20.0f, 0.0f}};
    guard_a.transform.set_position(0.0f, 0.0f);
    guard_a.combat.reset_for_spawn();
    guard_b = guard_a;
    guard_b.transform.set_position(40.0f, 0.0f);
    std::vector<mion::Actor*> actors = {&player, &guard_a, &guard_b};
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr, nullptr);
    EXPECT_TRUE(guard_a.patrol_state == mion::PatrolState::Alert
                || guard_a.patrol_state == mion::PatrolState::Chase);
    EXPECT_TRUE(guard_b.patrol_state == mion::PatrolState::Chase);
}

static void test_enemy_ai_boss_phase2_on_low_hp() {
    mion::EnemyAISystem sys;
    mion::Actor         player, boss;
    player.team     = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(200.0f, 100.0f);
    boss.team       = mion::Team::Enemy;
    boss.is_alive   = true;
    boss.ai_behavior = mion::AiBehavior::BossPhased;
    boss.aggro_range = 800.0f;
    boss.attack_range_ai = 40.0f;
    boss.stop_range_ai   = 20.0f;
    boss.move_speed      = 50.0f;
    boss.base_move_speed_at_spawn = 50.0f;
    boss.health.max_hp     = 200;
    boss.health.current_hp = 80;
    boss.boss_phase      = 1;
    boss.combat.reset_for_spawn();
    boss.transform.set_position(100.0f, 100.0f);
    std::vector<mion::Actor*> actors = {&player, &boss};
    sys.fixed_update(actors, 1.0f / 60.0f, nullptr, nullptr);
    EXPECT_EQ(boss.boss_phase, 2);
    EXPECT_GT(boss.move_speed, 60.0f);
}

static void test_enemy_ai_ranged_fires_projectile_when_cd_ready() {
    mion::EnemyAISystem sys;
    mion::Actor player, enemy;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.transform.set_position(220.0f, 100.0f);
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.ai_behavior = mion::AiBehavior::Ranged;
    enemy.aggro_range = 500.0f;
    enemy.ranged_fire_rate = 0.2f;
    enemy.ranged_fire_cd = 0.0f;
    enemy.ranged_proj_damage = 7;
    enemy.move_speed = 0.0f;
    enemy.stop_range_ai = 10.0f;
    enemy.attack_range_ai = 40.0f;
    enemy.transform.set_position(100.0f, 100.0f);
    enemy.combat.reset_for_spawn();
    std::vector<mion::Projectile> pr;
    std::vector<mion::Actor*> actors = { &player, &enemy };
    sys.fixed_update(actors, 1.0f / 30.0f, nullptr, &pr);
    EXPECT_FALSE(pr.empty());
    EXPECT_TRUE(pr[0].owner_team == mion::Team::Enemy);
}

static void test_enemy_type_name_new_types() {
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::Archer), "archer"), 0);
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::BossGrimjaw),
                          "boss_grimjaw"),
              0);
}

// ---------------------------------------------------------------------------
// Fase 7 — regras da dungeon (lógica extraída)
// ---------------------------------------------------------------------------
static void test_dungeon_rules_xp_and_spawn_budget() {
    EXPECT_EQ(mion::dungeon_rules::xp_per_enemy_kill(0), 14);
    EXPECT_EQ(mion::dungeon_rules::xp_per_enemy_kill(2), 26);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_normal(0), 3);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_normal(40), 80);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_stress(0), 1);
    EXPECT_EQ(mion::dungeon_rules::enemy_spawn_budget_stress(12), 12);
}

static void test_dungeon_rules_enemy_type_rotation() {
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(0) == mion::EnemyType::Skeleton);
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(1) == mion::EnemyType::Orc);
    EXPECT_TRUE(mion::dungeon_rules::enemy_type_for_spawn_index(2) == mion::EnemyType::Ghost);
    EXPECT_EQ(std::strcmp(mion::dungeon_rules::enemy_type_name(mion::EnemyType::Orc), "orc"), 0);
}

static void test_dungeon_rules_room_bounds_width_height() {
    mion::WorldBounds b0 = mion::dungeon_rules::room_bounds(0);
    EXPECT_NEAR(b0.max_x - b0.min_x, 1200.0f, 0.01f);
    EXPECT_NEAR(b0.max_y - b0.min_y, 900.0f, 0.01f);

    mion::WorldBounds b4 = mion::dungeon_rules::room_bounds(4);
    EXPECT_NEAR(b4.max_x - b4.min_x, 1400.0f, 0.01f);
    EXPECT_NEAR(b4.max_y - b4.min_y, 1050.0f, 0.01f);

    mion::WorldBounds b8 = mion::dungeon_rules::room_bounds(8);
    EXPECT_NEAR(b8.max_x - b8.min_x, 1600.0f, 0.01f);

    mion::WorldBounds b11 = mion::dungeon_rules::room_bounds(11);
    EXPECT_NEAR(b11.max_x - b11.min_x, 1800.0f, 0.01f);
    EXPECT_NEAR(b11.max_y - b11.min_y, 1350.0f, 0.01f);
}

static void test_dungeon_rules_room_template_ids() {
    EXPECT_EQ(mion::dungeon_rules::room_template(0), 5);
    EXPECT_EQ(mion::dungeon_rules::room_template(3), 4);
    EXPECT_EQ(mion::dungeon_rules::room_template(6), 4);
    EXPECT_EQ(mion::dungeon_rules::room_template(1), 1);
    EXPECT_EQ(mion::dungeon_rules::room_template(2), 2);
    EXPECT_EQ(mion::dungeon_rules::room_template(4), 0);
    EXPECT_EQ(mion::dungeon_rules::room_template(5), 1);
}

static void test_roomdefinition_compound_obstacle_helpers() {
    mion::RoomDefinition r;
    r.bounds = { 0.0f, 1000.0f, 0.0f, 800.0f };
    r.add_barrel_cluster(100.0f, 100.0f);
    EXPECT_EQ((int)r.obstacles.size(), 4);
    r.add_altar(200.0f, 200.0f);
    EXPECT_NEAR(r.obstacles.back().bounds.min_x, 160.0f, 0.01f);
    r.add_pillar_pair(400.0f, 300.0f, 0);
    EXPECT_EQ((int)r.obstacles.size(), 7);
    r.obstacles.clear();
    r.add_wall_L(150.0f, 150.0f, 0);
    EXPECT_EQ((int)r.obstacles.size(), 2);
}

// ---------------------------------------------------------------------------
// Fase 8 — partículas (spawn + update; render coberto por render_stress)
// ---------------------------------------------------------------------------
static void test_particles_spawn_and_update_reduces_count() {
    std::mt19937 rng(42);
    mion::SimpleParticleSystem ps;
    ps.spawn_burst(0.0f, 0.0f, 20, 255, 0, 0, 10.0f, 12.0f, rng);
    EXPECT_TRUE(ps.live_particle_count() > 0);
    int n0 = ps.live_particle_count();
    for (int i = 0; i < 120; ++i)
        ps.update(0.05f);
    EXPECT_TRUE(ps.live_particle_count() <= n0);
}

// ---------------------------------------------------------------------------
// Fase 9 — áudio integração (opcional, só com env)
// ---------------------------------------------------------------------------
static void test_audio_integration_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }
    audio.set_master_volume(0.01f);
    audio.play_sfx(mion::SoundId::UiConfirm, 0.01f);
    for (int i = 0; i < 5; ++i)
        audio.tick_music();
    audio.shutdown();
    SDL_Quit();
    EXPECT_TRUE(true);
}

static void test_audio_music_state_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }

    audio.set_master_volume(0.01f);
    audio.set_music_state(mion::MusicState::Town);
    for (int i = 0; i < 8; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::Town);

    audio.set_music_state(mion::MusicState::DungeonCombat);
    for (int i = 0; i < 220; ++i) {
        audio.tick_music();
        SDL_Delay(10);
    }
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::DungeonCombat);

    audio.set_music_state(mion::MusicState::None);
    for (int i = 0; i < 8; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::None);

    audio.shutdown();
    SDL_Quit();
}

static void test_audio_ambient_and_spatial_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }

    audio.set_master_volume(0.01f);
    audio.play_ambient(mion::AmbientId::DungeonDrips, 0.1f);
    audio.play_ambient(mion::AmbientId::DungeonTorch, 0.08f);
    for (int i = 0; i < 6; ++i)
        audio.tick_music();
    audio.stop_ambient(mion::AmbientId::DungeonTorch);
    audio.play_sfx_pitched(mion::SoundId::UiConfirm, 0.05f);
    audio.play_sfx_at(mion::SoundId::Hit, 100.0f, 100.0f, 90.0f, 90.0f, 500.0f, 0.2f);
    audio.stop_all_ambient();
    for (int i = 0; i < 6; ++i)
        audio.tick_music();

    audio.shutdown();
    SDL_Quit();
    EXPECT_TRUE(true);
}

static void test_texture_manifest_inventory_optional_env() {
    if (!std::getenv("MION_TEXTURE_INTEGRATION_TESTS"))
        return;

    std::string manifest_path = "tools/texture_manifest.json";
    std::ifstream in(manifest_path);
    if (!in.good()) {
        manifest_path = "../tools/texture_manifest.json";
        in.clear();
        in.open(manifest_path);
    }
    if (!in.good()) {
        EXPECT_TRUE(false);
        return;
    }

    std::string manifest((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());

    size_t found_paths = 0;
    size_t pos = 0;
    while (true) {
        size_t k = manifest.find("\"path\"", pos);
        if (k == std::string::npos) break;
        size_t c = manifest.find(':', k);
        if (c == std::string::npos) break;
        size_t q1 = manifest.find('"', c + 1);
        if (q1 == std::string::npos) break;
        size_t q2 = manifest.find('"', q1 + 1);
        if (q2 == std::string::npos) break;

        std::string rel = manifest.substr(q1 + 1, q2 - (q1 + 1));
        FILE* f = std::fopen(rel.c_str(), "rb");
        if (!f) {
            std::string alt = "../" + rel;
            f = std::fopen(alt.c_str(), "rb");
        }
        EXPECT_TRUE(f != nullptr);
        if (f) std::fclose(f);

        ++found_paths;
        pos = q2 + 1;
    }

    EXPECT_TRUE(found_paths > 0);
}

// ---------------------------------------------------------------------------
// Fase 7b — smoke DungeonScene (opcional MION_DUNGEON_SMOKE)
// ---------------------------------------------------------------------------
static void test_ini_sections_with_prefix_sorted() {
    mion::IniData d;
    d.sections["arena.obstacle.2"]["k"] = "1";
    d.sections["arena.obstacle.0"]["k"] = "1";
    d.sections["arena.obstacle.1"]["k"] = "1";
    d.sections["arena"]["x"]              = "1";
    d.sections["other"]["k"]              = "1";
    auto v = d.sections_with_prefix("arena.obstacle");
    EXPECT_EQ((int)v.size(), 3);
    EXPECT_TRUE(v[0] == "arena.obstacle.0");
    EXPECT_TRUE(v[1] == "arena.obstacle.1");
    EXPECT_TRUE(v[2] == "arena.obstacle.2");
}

static void test_room_loader_normalized_obstacles() {
    mion::IniData d;
    d.sections["arena"]["ini_obstacles"] = "1";
    d.sections["arena.obstacle.0"]["name"] = "box";
    d.sections["arena.obstacle.0"]["nx1"] = "0";
    d.sections["arena.obstacle.0"]["ny1"] = "0";
    d.sections["arena.obstacle.0"]["nx2"] = "0.1";
    d.sections["arena.obstacle.0"]["ny2"] = "0.2";
    mion::RoomDefinition room;
    mion::WorldBounds  b{0.0f, 1000.0f, 0.0f, 800.0f};
    EXPECT_TRUE(mion::load_room_obstacles_from_ini(room, d, b, "arena"));
    EXPECT_EQ((int)room.obstacles.size(), 1);
    EXPECT_NEAR(room.obstacles[0].bounds.min_x, 0.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.max_x, 100.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.min_y, 0.0f, 0.01f);
    EXPECT_NEAR(room.obstacles[0].bounds.max_y, 160.0f, 0.01f);
}

static void test_room_loader_ini_obstacles_off_skips() {
    mion::IniData d;
    d.sections["arena"]["ini_obstacles"] = "0";
    d.sections["arena.obstacle.0"]["nx1"] = "0";
    d.sections["arena.obstacle.0"]["ny1"] = "0";
    d.sections["arena.obstacle.0"]["nx2"] = "1";
    d.sections["arena.obstacle.0"]["ny2"] = "1";
    mion::RoomDefinition room;
    room.obstacles.push_back({ "keep", {0.0f, 10.0f, 0.0f, 10.0f}, "", 0.0f, 0.0f });
    mion::WorldBounds b{0.0f, 100.0f, 0.0f, 100.0f};
    EXPECT_FALSE(mion::load_room_obstacles_from_ini(room, d, b, "arena"));
    EXPECT_EQ((int)room.obstacles.size(), 1);
}

static void test_dungeon_rules_room_template_id_names() {
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(0), "arena") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(1), "corredor") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(4), "boss_arena") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(5), "intro") == 0);
    EXPECT_TRUE(std::strcmp(mion::dungeon_rules::room_template_id_name(99), "intro") == 0);
}

static void test_save_gold_quest_roundtrip_file() {
    mion::SaveData d;
    d.gold        = 55;
    d.quest_state.set(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::Completed);
    const char* path = "mion_save_town_quest_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.gold, 55);
    EXPECT_TRUE(out.quest_state.is(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::Completed));
    std::remove(path);
}

static void test_shop_try_buy_hp_potion() {
    mion::Actor player;
    player.gold                 = 100;
    player.health               = {50, 50};
    mion::ShopInventory shop;
    shop.items.push_back({"HP Potion", mion::ShopItemType::HpPotion, 20, 25});
    shop.selected_index = 0;
    EXPECT_TRUE(mion::ShopSystem::try_buy(player, shop, 0));
    EXPECT_EQ(player.gold, 80);
    EXPECT_EQ(player.health.current_hp, 50);
}

static void test_shop_try_buy_rejects_insufficient_gold() {
    mion::Actor player;
    player.gold = 5;
    mion::ShopInventory shop;
    shop.items.push_back({"HP Potion", mion::ShopItemType::HpPotion, 20, 25});
    EXPECT_FALSE(mion::ShopSystem::try_buy(player, shop, 0));
    EXPECT_EQ(player.gold, 5);
}

static void test_ui_list_nav_skips_disabled() {
    mion::ui::List L;
    L.items     = {"A", "B", "C"};
    L.disabled  = {false, true, false};
    L.selected  = 0;
    L.nav_down();
    EXPECT_EQ(L.selected, 2);
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
    L.nav_up();
    EXPECT_EQ(L.selected, 2);
}

static void test_ui_list_all_disabled_stays() {
    mion::ui::List L;
    L.items    = {"X", "Y"};
    L.disabled = {true, true};
    L.selected = 0;
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
}

static void test_ui_progress_bar_normalized_fill_clamps() {
    mion::ui::ProgressBar bar;
    bar.value = -0.5f;
    EXPECT_NEAR(bar.normalized_fill(), 0.0f, 0.0001f);
    bar.value = 0.37f;
    EXPECT_NEAR(bar.normalized_fill(), 0.37f, 0.0001f);
    bar.value = 1.7f;
    EXPECT_NEAR(bar.normalized_fill(), 1.0f, 0.0001f);
}

static void test_ui_list_wraps_when_none_disabled() {
    mion::ui::List L;
    L.items    = {"a", "b"};
    L.selected = 0;
    L.nav_up();
    EXPECT_EQ(L.selected, 1);
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
}

static void test_quest_state_int_mapping() {
    EXPECT_EQ(mion::QuestState::status_to_int(mion::QuestStatus::Rewarded), 3);
    EXPECT_TRUE(mion::QuestState::int_to_status(99) == mion::QuestStatus::Rewarded);
}

static void test_dungeon_scene_smoke_optional() {
    if (!std::getenv("MION_DUNGEON_SMOKE"))
        return;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return;
    SDL_Window* win = SDL_CreateWindow("t", 32, 32, SDL_WINDOW_HIDDEN);
    if (!win) {
        SDL_Quit();
        return;
    }
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) {
        SDL_DestroyWindow(win);
        SDL_Quit();
        return;
    }
    {
        mion::DungeonScene scene;
        scene.set_renderer(ren);
        scene.viewport_w = 320;
        scene.viewport_h = 240;
        scene.enter();
        mion::InputState in;
        scene.fixed_update(0.016f, in);
        scene.exit();
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    EXPECT_TRUE(true);
}

// ---------------------------------------------------------------------------
void run_extended_plan_tests() {
    run("Stamina.ConsumeRegen", test_stamina_consume_clamp_and_regen_delay);
    run("Stamina.CapMax", test_stamina_tick_caps_at_max);
    run("StatusEffect.HasApply", test_status_effect_has_and_apply_replace);
    run("StatusEffect.SlowMin", test_status_effect_slow_multiplier_min);
    run("StatusEffect.ClearExpired", test_status_effect_clear_expired);
    run("StatusEffect.SlotOverwrite", test_status_effect_full_slots_overwrites_slot0);
    run("Progression.MultiXp", test_progression_add_xp_multi_level);
    run("Progression.PickUpgrade", test_progression_pick_upgrades_reduce_pending);
    run("Ini.GetString", test_ini_get_string);
    run("Progression.IniXpCurve", test_progression_ini_xp_curve);
    run("Player.IniApply", test_player_ini_applies_fields);
    run("Talents.IniDisplay", test_talents_ini_display_name);
    run("Spell.EffectiveScaling", test_spell_def_effective_scaling);
    run("Ini.StringFile", test_ini_load_get_string_from_file);
    run("Ini.PrefixSections", test_ini_sections_with_prefix_sorted);
    run("RoomLoader.Normalized", test_room_loader_normalized_obstacles);
    run("RoomLoader.IniOff", test_room_loader_ini_obstacles_off_skips);
    run("DungeonRules.TemplateIds", test_dungeon_rules_room_template_id_names);
    run("Enemy.IniApply", test_apply_enemy_ini_section_overrides);
    run("Spell.IniApply", test_apply_spell_ini_section_per_level_fields);
    run("Progression.IniLevelUp", test_progression_ini_custom_level_up_bonuses);
    run("Player.IniClampMana", test_player_ini_clamps_mana_and_stamina_max);
    run("Talents.IniCost", test_talent_ini_cost_per_level_affects_unlock);
    run("DataRepo.InisOptional", test_data_repo_inis_parse_if_present);
    run("PlayerAction.RangedDamageField", test_player_action_ranged_uses_actor_ranged_damage);
    run("Actor.EffectiveSpeed", test_actor_effective_move_speed_player_bonus_and_slow);
    run("Actor.IsDashing", test_actor_is_dashing);
    run("Resource.TickAlive", test_resource_system_ticks_mana_and_stamina);
    run("Resource.SkipDead", test_resource_system_skips_dead);
    run("Movement.ResetKnockback", test_movement_system_resets_moving_and_knockback);
    run("StatusSys.Poison", test_status_effect_system_poison_ticks);
    run("Projectile.Lifetime", test_projectile_expires_on_lifetime);
    run("Projectile.OutOfBounds", test_projectile_despawns_outside_room);
    run("Projectile.HitEnemy", test_projectile_hits_enemy_and_applies_damage);
    run("Projectile.NoFriendly", test_projectile_skips_same_team);
    run("Projectile.Obstacle", test_projectile_blocked_by_obstacle);
    run("PlayerAction.Move", test_player_action_moves_and_sets_facing);
    run("PlayerAction.MeleeNoStam", test_player_action_melee_requires_stamina);
    run("PlayerAction.MeleeOk", test_player_action_melee_begins_attack_with_stamina);
    run("PlayerAction.Ranged", test_player_action_ranged_spawns_and_damage_formula);
    run("PlayerAction.Bolt", test_player_action_bolt_consumes_mana);
    run("PlayerAction.Nova", test_player_action_nova_hits_enemy_list);
    run("PlayerAction.Dash", test_player_action_dash_consumes_stamina);
    run("PlayerAction.Stun", test_player_action_stun_blocks_movement);
    run("PlayerAction.Parry", test_player_action_parry_sets_window);
    run("NavGrid.Obstacle", test_nav_grid_obstacle_blocks_tile);
    run("NavGrid.TileClamp", test_nav_grid_world_to_tile_clamp);
    run("Path.SameTile", test_pathfinder_same_tile_valid_empty_waypoints);
    run("Path.AroundWall", test_pathfinder_find_path_around_wall);
    run("Path.Unreachable", test_pathfinder_unreachable_goal);
    run("Path.AdvanceAPI", test_path_advance_and_done);
    run("Anim.LoopDeath", test_anim_player_loop_and_non_loop_finished);
    run("Anim.NoRestartMid", test_anim_player_play_same_clip_no_restart_until_finished);
    run("Anim.NullFrame", test_anim_player_current_frame_null_when_empty);
    run("Anim.PunyRow", test_anim_player_puny_dir_row_updates_y);
    run("Camera.FollowClamp", test_camera_follow_clamps);
    run("Camera.WorldToScreen", test_camera_world_to_screen_no_shake);
    run("Camera.ShakeDecay", test_camera_shake_steps_down);
    run("EnemyAI.Chase", test_enemy_ai_chases_player_without_pathfinder);
    run("EnemyAI.StopRange", test_enemy_ai_respects_stop_range_no_move);
    run("EnemyAI.AttackRange", test_enemy_ai_begins_attack_when_in_range);
    run("EnemyDef.BossGrimjaw", test_enemy_def_boss_grimjaw_boss_phased_and_gold);
    run("Enemy.AiBehaviorParse", test_parse_ai_behavior_string_variants);
    run("EnemyAI.RangedProjectile", test_enemy_ai_ranged_fires_projectile_when_cd_ready);
    run("RunStats.Reset", test_run_stats_reset_clears_fields);
    run("Difficulty.SpawnMult", test_difficulty_spawn_and_multipliers);
    run("Save.MetaRun", test_save_roundtrip_meta_run_fields);
    run("EnemyAI.PatrolAlert", test_enemy_ai_patrol_alert_neighbor_chase);
    run("EnemyAI.BossPhase2", test_enemy_ai_boss_phase2_on_low_hp);
    run("DungeonRules.EnemyNames", test_enemy_type_name_new_types);
    run("DungeonRules.XpSpawn", test_dungeon_rules_xp_and_spawn_budget);
    run("DungeonRules.Types", test_dungeon_rules_enemy_type_rotation);
    run("DungeonRules.RoomBounds", test_dungeon_rules_room_bounds_width_height);
    run("DungeonRules.RoomTemplate", test_dungeon_rules_room_template_ids);
    run("Room.CompoundObstacles", test_roomdefinition_compound_obstacle_helpers);
    run("SceneFader.FadeOutBlack", test_scene_fader_fade_out_holds_black);
    run("SceneFader.FadeInClear", test_scene_fader_fade_in_goes_clear);
    run("FloatText.TickErase", test_floating_text_tick_erases_dead);
    run("Audio.Attenuation", test_sfx_distance_attenuation_quadratic);
    run("Particles.SpawnUpdate", test_particles_spawn_and_update_reduces_count);
    run("Audio.IntegrationEnv", test_audio_integration_optional_env);
    run("Audio.MusicStateEnv", test_audio_music_state_optional_env);
    run("Audio.AmbientSpatialEnv", test_audio_ambient_and_spatial_optional_env);
    run("Texture.ManifestEnv", test_texture_manifest_inventory_optional_env);
    run("Dungeon.SmokeEnv", test_dungeon_scene_smoke_optional);
    run("Save.GoldQuest", test_save_gold_quest_roundtrip_file);
    run("Shop.BuyHp", test_shop_try_buy_hp_potion);
    run("Shop.NoGold", test_shop_try_buy_rejects_insufficient_gold);
    run("Quest.IntMap", test_quest_state_int_mapping);
    run("UI.ListSkipDisabled", test_ui_list_nav_skips_disabled);
    run("UI.ListAllDisabled", test_ui_list_all_disabled_stays);
    run("UI.ProgressBarClamp", test_ui_progress_bar_normalized_fill_clamps);
    run("UI.ListWrap", test_ui_list_wraps_when_none_disabled);
}
