#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include <SDL3/SDL.h>

#include "../core/run_stats.hpp"
#include "../core/sprite.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../world/dungeon_rules.hpp"
#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "pathfinder.hpp"

namespace mion {

struct EnemySpawnResult {
    bool  boss_phase2_triggered = false;
    bool  boss_intro_pending    = false;
    float boss_intro_timer      = 0.0f;
};

struct EnemySpawner {
    static int spawn_budget(int room_index, bool stress_mode, int requested_enemy_count,
                            const DifficultyLevel* difficulty) {
        int budget = stress_mode
            ? dungeon_rules::enemy_spawn_budget_stress(requested_enemy_count)
            : dungeon_rules::enemy_spawn_budget_normal(room_index);
        if (!stress_mode && difficulty)
            budget = difficulty_adjust_spawn_budget(budget, *difficulty);
        return budget;
    }

    static EnemySpawnResult spawn_for_budget(std::vector<Actor>& enemies,
                                             const RoomDefinition& room,
                                             const Tilemap& tilemap,
                                             const Pathfinder& pathfinder,
                                             const Actor& player,
                                             const EnemyDef* enemy_defs,
                                             TextureCache* tex_cache,
                                             int room_index,
                                             int budget,
                                             bool stress_mode,
                                             const DifficultyLevel* difficulty) {
        enemies.clear();

        EnemySpawnResult result{};
        if (!stress_mode && room_index == 3) {
            enemies.reserve(1);
            const WorldBounds& bounds = room.bounds;
            const float        width  = bounds.max_x - bounds.min_x;
            const float        height = bounds.max_y - bounds.min_y;
            spawn_enemy(enemies, EnemyType::BossGrimjaw,
                        bounds.min_x + width * 0.625f, bounds.min_y + height * 0.5f,
                        "Grimjaw", enemy_defs, tex_cache, stress_mode, difficulty);
            result.boss_intro_pending = true;
            return result;
        }

        enemies.reserve(static_cast<size_t>(std::max(budget, 4)));
        if (budget <= 3)
            spawn_default_enemies(enemies, room, enemy_defs, tex_cache, stress_mode, difficulty,
                                  budget);
        else
            spawn_stress_enemies(enemies, tilemap, pathfinder, player, enemy_defs,
                                 tex_cache, stress_mode, difficulty, budget);
        return result;
    }

private:
    static void spawn_enemy(std::vector<Actor>& enemies,
                            EnemyType type,
                            float x,
                            float y,
                            const char* name,
                            const EnemyDef* enemy_defs,
                            TextureCache* tex_cache,
                            bool stress_mode,
                            const DifficultyLevel* difficulty) {
        const EnemyDef& def = enemy_defs[static_cast<int>(type)];
        Actor           actor;
        actor.name               = name;
        actor.team               = Team::Enemy;
        actor.enemy_type         = type;
        actor.attack_damage      = def.damage;
        actor.aggro_range        = def.aggro_range;
        actor.attack_range_ai    = def.attack_range;
        actor.stop_range_ai      = def.stop_range;
        actor.move_speed         = def.move_speed;
        actor.base_move_speed_at_spawn = def.move_speed;
        actor.knockback_friction = def.knockback_friction;
        actor.collision          = def.collision;
        actor.hurt_box           = def.hurt_box;
        actor.melee_hit_box      = def.melee_hit_box;
        actor.health             = { def.max_hp, def.max_hp };
        if (difficulty && !stress_mode) {
            float hp_mult = 1.0f;
            float dmg_mult = 1.0f;
            difficulty_enemy_multipliers(*difficulty, hp_mult, dmg_mult);
            int hp = static_cast<int>(std::lround(static_cast<float>(def.max_hp) * hp_mult));
            int ad = static_cast<int>(std::lround(static_cast<float>(def.damage) * dmg_mult));
            actor.health        = { std::max(1, hp), std::max(1, hp) };
            actor.attack_damage = std::max(1, ad);
        }
        actor.derived.melee_damage_final = actor.attack_damage;
        actor.combat.attack_startup_duration_seconds  = def.startup_sec;
        actor.combat.attack_active_duration_seconds   = def.active_sec;
        actor.combat.attack_recovery_duration_seconds = def.recovery_sec;
        actor.combat.reset_for_spawn();
        actor.transform.set_position(x, y);
        actor.is_alive           = true;
        actor.was_alive          = true;
        actor.knockback_vx       = 0.0f;
        actor.knockback_vy       = 0.0f;
        actor.sprite_scale       = def.sprite_scale;
        actor.path_replan_timer  = static_cast<float>(enemies.size()) * 0.1f;
        actor.sprite_sheet = (tex_cache && def.sprite_sheet_path && def.sprite_sheet_path[0])
            ? static_cast<void*>(tex_cache->load(def.sprite_sheet_path))
            : nullptr;
        if (actor.sprite_sheet)
            actor.anim.build_puny_clips(def.dir_row, def.frame_fps);

        actor.ai_behavior        = def.ai_behavior;
        actor.ranged_fire_rate   = def.ranged_fire_rate;
        actor.ranged_keep_dist   = def.ranged_keep_dist;
        actor.ranged_proj_damage = def.ranged_proj_damage;
        actor.ranged_fire_cd     = def.ranged_fire_rate * 0.35f;
        actor.is_elite           = def.is_elite_base;
        actor.boss_phase         = 1;
        actor.boss_charging      = false;
        actor.boss_charge_cd     = 1.2f;
        actor.patrol_state       = PatrolState::Patrol;
        actor.alert_timer        = 0.0f;
        actor.patrol_wp_index    = 0;
        actor.patrol_waypoints.clear();

        if (type == EnemyType::PatrolGuard)
            actor.patrol_waypoints = {{x - 90.0f, y}, {x + 90.0f, y}, {x, y - 70.0f}, {x, y + 70.0f}};
        if (type == EnemyType::BossGrimjaw)
            actor.patrol_waypoints = {{x - 180.0f, y - 130.0f},
                                      {x + 180.0f, y - 130.0f},
                                      {x + 180.0f, y + 130.0f},
                                      {x - 180.0f, y + 130.0f}};

        actor.footstep_prev_x     = x;
        actor.footstep_prev_y     = y;
        actor.footstep_accum_dist = 0.0f;
        enemies.push_back(std::move(actor));
    }

    static void spawn_default_enemies(std::vector<Actor>& enemies,
                                      const RoomDefinition& room,
                                      const EnemyDef* enemy_defs,
                                      TextureCache* tex_cache,
                                      bool stress_mode,
                                      const DifficultyLevel* difficulty,
                                      int count) {
        const WorldBounds& bounds = room.bounds;
        const float        width  = bounds.max_x - bounds.min_x;
        const float        height = bounds.max_y - bounds.min_y;
        if (count >= 1)
            spawn_enemy(enemies, EnemyType::Skeleton, bounds.min_x + width * 0.56f,
                        bounds.min_y + height * 0.5f, "skeleton_01",
                        enemy_defs, tex_cache, stress_mode, difficulty);
        if (count >= 2)
            spawn_enemy(enemies, EnemyType::Orc, bounds.min_x + width * 0.69f,
                        bounds.min_y + height * 0.33f, "orc_01",
                        enemy_defs, tex_cache, stress_mode, difficulty);
        if (count >= 3)
            spawn_enemy(enemies, EnemyType::Ghost, bounds.min_x + width * 0.44f,
                        bounds.min_y + height * 0.25f, "ghost_01",
                        enemy_defs, tex_cache, stress_mode, difficulty);
    }

    static void spawn_stress_enemies(std::vector<Actor>& enemies,
                                     const Tilemap& tilemap,
                                     const Pathfinder& pathfinder,
                                     const Actor& player,
                                     const EnemyDef* enemy_defs,
                                     TextureCache* tex_cache,
                                     bool stress_mode,
                                     const DifficultyLevel* difficulty,
                                     int count) {
        constexpr float player_clear_radius_sq = 180.0f * 180.0f;
        constexpr int   step_tiles             = 2;
        int             spawned                = 0;

        for (int row = 1; row < tilemap.rows - 1 && spawned < count; row += step_tiles) {
            for (int col = 1; col < tilemap.cols - 1 && spawned < count; col += step_tiles) {
                if (!pathfinder.nav.is_walkable(col, row))
                    continue;

                const float x = (col + 0.5f) * static_cast<float>(tilemap.tile_size);
                const float y = (row + 0.5f) * static_cast<float>(tilemap.tile_size);
                const float dx = x - player.transform.x;
                const float dy = y - player.transform.y;
                if (dx * dx + dy * dy < player_clear_radius_sq)
                    continue;

                const EnemyType type = dungeon_rules::enemy_type_for_spawn_index(spawned);
                char            name[32];
                SDL_snprintf(name, sizeof(name), "%s_%03d",
                             dungeon_rules::enemy_type_name(type), spawned + 1);
                spawn_enemy(enemies, type, x, y, name, enemy_defs, tex_cache,
                            stress_mode, difficulty);
                ++spawned;
            }
        }

        if (spawned < count) {
            SDL_Log("DungeonScene: stress spawn truncado em %d inimigos (pedido=%d)",
                    spawned, count);
        }
    }
};

} // namespace mion
