#pragma once
#include <cctype>
#include <string>

#include "../components/collision.hpp"
#include "../core/ini_loader.hpp"

namespace mion {

// AI decision style assigned to each enemy type.
enum class AiBehavior {
    Melee,
    Ranged,
    Patrol,
    Elite,
    BossPhased,
};

// Unique identifier for each enemy variant.
enum class EnemyType {
    Skeleton,
    Orc,
    Ghost,
    Archer,
    PatrolGuard,
    EliteSkeleton,
    BossGrimjaw,
    Count
};

inline constexpr int kEnemyTypeCount = static_cast<int>(EnemyType::Count);

// Static definition of an enemy: stats, hitboxes, sprite, and AI config.
// Loaded from compiled-in defaults and overridable via data/enemies.ini.
struct EnemyDef {
    EnemyType    type;
    int          max_hp;
    float        move_speed;
    float        aggro_range;
    float        attack_range;
    float        stop_range;
    int          damage;
    float        knockback_friction;
    CollisionBox collision;
    HurtBox      hurt_box;
    MeleeHitBox  melee_hit_box;
    float        startup_sec;
    float        active_sec;
    float        recovery_sec;
    const char*  sprite_sheet_path;
    float        sprite_scale;
    float        frame_fps;
    int          dir_row;
    int          gold_drop_min = 1;
    int          gold_drop_max = 4;
    AiBehavior   ai_behavior           = AiBehavior::Melee;
    float        ranged_fire_rate      = 1.5f;
    float        ranged_keep_dist      = 200.0f;
    int          ranged_proj_damage    = 8;
    bool         is_elite_base         = false;
};

inline AiBehavior parse_ai_behavior_string(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s)
        lower.push_back((char)std::tolower((unsigned char)c));
    if (lower == "ranged")
        return AiBehavior::Ranged;
    if (lower == "patrol")
        return AiBehavior::Patrol;
    if (lower == "elite")
        return AiBehavior::Elite;
    if (lower == "boss_phased" || lower == "bossphased")
        return AiBehavior::BossPhased;
    return AiBehavior::Melee;
}

inline const EnemyDef& get_enemy_def(EnemyType type) {
    static const EnemyDef DEFS[] = {
        { EnemyType::Skeleton, 60, 80.0f, 400.0f, 45.0f, 30.0f, 8, 0.72f,
          {16.0f, 16.0f}, {14.0f, 14.0f}, {20.0f, 12.0f, 22.0f},
          0.10f, 0.15f, 0.25f,
          "assets/Puny-Characters/Puny-Characters/Soldier-Red.png", 2.0f, 8.0f, 0,
          1, 3, AiBehavior::Melee, 1.5f, 200.0f, 8, false },
        { EnemyType::Orc, 120, 55.0f, 300.0f, 50.0f, 35.0f, 18, 0.68f,
          {20.0f, 18.0f}, {18.0f, 16.0f}, {26.0f, 14.0f, 28.0f},
          0.15f, 0.20f, 0.35f,
          "assets/Puny-Characters/Puny-Characters/Orc-Grunt.png", 2.0f, 8.0f, 0,
          3, 6, AiBehavior::Melee, 1.5f, 200.0f, 8, false },
        { EnemyType::Ghost, 35, 130.0f, 500.0f, 40.0f, 25.0f, 5, 0.80f,
          {14.0f, 14.0f}, {12.0f, 12.0f}, {18.0f, 10.0f, 20.0f},
          0.08f, 0.10f, 0.20f,
          "assets/Puny-Characters/Puny-Characters/Mage-Cyan.png", 2.0f, 8.0f, 0,
          2, 4, AiBehavior::Melee, 1.5f, 200.0f, 8, false },
        { EnemyType::Archer, 40, 70.0f, 450.0f, 48.0f, 34.0f, 8, 0.75f,
          {14.0f, 14.0f}, {12.0f, 12.0f}, {18.0f, 10.0f, 20.0f},
          0.10f, 0.12f, 0.22f,
          "assets/Puny-Characters/Puny-Characters/Mage-Purple.png", 2.0f, 8.0f, 0,
          2, 5, AiBehavior::Ranged, 1.5f, 200.0f, 10, false },
        { EnemyType::PatrolGuard, 80, 65.0f, 280.0f, 50.0f, 32.0f, 12, 0.70f,
          {18.0f, 18.0f}, {16.0f, 16.0f}, {22.0f, 12.0f, 24.0f},
          0.12f, 0.18f, 0.30f,
          "assets/Puny-Characters/Puny-Characters/Soldier-Blue.png", 2.0f, 8.0f, 0,
          2, 6, AiBehavior::Patrol, 0.0f, 0.0f, 0, false },
        { EnemyType::EliteSkeleton, 120, 96.0f, 420.0f, 45.0f, 30.0f, 12, 0.72f,
          {16.0f, 16.0f}, {14.0f, 14.0f}, {20.0f, 12.0f, 22.0f},
          0.10f, 0.15f, 0.25f,
          "assets/Puny-Characters/Puny-Characters/Soldier-Red.png", 2.4f, 8.0f, 0,
          5, 10, AiBehavior::Elite, 0.0f, 0.0f, 0, true },
        { EnemyType::BossGrimjaw, 320, 70.0f, 600.0f, 52.0f, 36.0f, 22, 0.65f,
          {22.0f, 20.0f}, {20.0f, 18.0f}, {28.0f, 14.0f, 30.0f},
          0.15f, 0.20f, 0.35f,
          "assets/Puny-Characters/Puny-Characters/Orc-Grunt.png", 2.8f, 8.0f, 0,
          25, 35, AiBehavior::BossPhased, 0.0f, 0.0f, 0, false },
    };
    const int i = static_cast<int>(type);
    if (i < 0 || i >= kEnemyTypeCount)
        return DEFS[0];
    return DEFS[i];
}

// Patches a single EnemyDef from the named INI section.
// sprite_path_storage must outlive def if a new path is loaded.
inline void apply_enemy_ini_section(const IniData& d, const std::string& sec, EnemyDef& def,
                                    std::string* sprite_path_storage) {
    def.max_hp = d.get_int(sec, "max_hp", def.max_hp);
    def.move_speed = d.get_float(sec, "move_speed", def.move_speed);
    def.aggro_range = d.get_float(sec, "aggro_range", def.aggro_range);
    def.attack_range = d.get_float(sec, "attack_range", def.attack_range);
    def.stop_range = d.get_float(sec, "stop_range", def.stop_range);
    def.damage = d.get_int(sec, "damage", def.damage);
    def.knockback_friction =
        d.get_float(sec, "knockback_friction", def.knockback_friction);
    def.startup_sec = d.get_float(sec, "startup_sec", def.startup_sec);
    def.active_sec = d.get_float(sec, "active_sec", def.active_sec);
    def.recovery_sec = d.get_float(sec, "recovery_sec", def.recovery_sec);
    std::string sheet = d.get_string(sec, "sprite_sheet_path", "");
    if (!sheet.empty() && sprite_path_storage) {
        *sprite_path_storage = std::move(sheet);
        def.sprite_sheet_path = sprite_path_storage->c_str();
    }
    def.sprite_scale = d.get_float(sec, "sprite_scale", def.sprite_scale);
    def.frame_fps = d.get_float(sec, "frame_fps", def.frame_fps);
    def.dir_row = d.get_int(sec, "dir_row", def.dir_row);
    def.gold_drop_min = d.get_int(sec, "gold_drop_min", def.gold_drop_min);
    def.gold_drop_max = d.get_int(sec, "gold_drop_max", def.gold_drop_max);
    if (def.gold_drop_max < def.gold_drop_min)
        def.gold_drop_max = def.gold_drop_min;

    std::string ab = d.get_string(sec, "ai_behavior", "");
    if (!ab.empty())
        def.ai_behavior = parse_ai_behavior_string(ab);
    def.ranged_fire_rate   = d.get_float(sec, "ranged_fire_rate", def.ranged_fire_rate);
    def.ranged_keep_dist   = d.get_float(sec, "ranged_keep_dist", def.ranged_keep_dist);
    def.ranged_proj_damage = d.get_int(sec, "ranged_proj_damage", def.ranged_proj_damage);
    def.is_elite_base      = d.get_int(sec, "is_elite_base", def.is_elite_base ? 1 : 0) != 0;
}

} // namespace mion
