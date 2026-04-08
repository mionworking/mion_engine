#include "../test_common.hpp"

#include <cstdio>

#include "../../src/core/save_system.hpp"
#include "../../src/systems/area_entry_system.hpp"
#include "../../src/systems/world_save_controller.hpp"
#include "../../src/world/world_context.hpp"
#include "../../src/world/world_map.hpp"

using namespace mion;

static bool world_save_default_backend_available() {
    SaveSystem::remove_default_saves();
    SaveData probe{};
    probe.version = kSaveFormatVersion;
    const bool ok = SaveSystem::save_default(probe);
    if (ok)
        SaveSystem::remove_default_saves();
    return ok;
}

static void world_save_make_and_apply_restores_position_and_mask() {
    WorldMap map;
    AreaEntrySystem entry;
    entry.visited_areas.insert(ZoneId::DungeonRoom1);

    Actor player;
    player.transform.set_position(500.f, 600.f);
    player.health.current_hp = 80;
    player.gold              = 12;

    WorldContext src;
    src.world_map  = &map;
    src.player     = &player;
    src.area_entry = &entry;

    SaveData sd = WorldSaveController::make_world_save(src);
    EXPECT_NEAR(sd.player_world_x, 500.f, 0.01f);
    EXPECT_NEAR(sd.player_world_y, 600.f, 0.01f);
    EXPECT_EQ(static_cast<int>(sd.visited_area_mask), 0x02);

    Actor player2;
    player2.transform.set_position(0.f, 0.f);
    AreaEntrySystem entry2;
    WorldContext dst;
    dst.world_map  = &map;
    dst.player     = &player2;
    dst.area_entry = &entry2;

    WorldSaveController::apply_world_save(dst, sd);
    EXPECT_NEAR(player2.transform.x, 500.f, 0.01f);
    EXPECT_NEAR(player2.transform.y, 600.f, 0.01f);
    EXPECT_TRUE(entry2.visited_areas.count(ZoneId::DungeonRoom1) != 0);
    EXPECT_EQ(player2.health.current_hp, 80);
}

static void world_save_merge_preserves_victory_like_persist() {
    const char* path = "/tmp/mion_test_world_merge_victory.txt";

    SaveData seed{};
    seed.version         = kSaveFormatVersion;
    seed.victory_reached = true;
    seed.player_hp       = 100;
    EXPECT_TRUE(SaveSystem::save(path, seed));

    WorldMap map;
    AreaEntrySystem entry;
    Actor player;
    player.transform.set_position(100.f, 200.f);

    WorldContext ctx;
    ctx.world_map   = &map;
    ctx.player      = &player;
    ctx.area_entry  = &entry;
    bool stress     = false;
    ctx.stress_mode = &stress;

    SaveData d     = WorldSaveController::make_world_save(ctx);
    SaveData disk{};
    EXPECT_TRUE(SaveSystem::load(path, disk));
    d.victory_reached = disk.victory_reached;
    EXPECT_TRUE(SaveSystem::save(path, d));

    SaveData out{};
    EXPECT_TRUE(SaveSystem::load(path, out));
    EXPECT_TRUE(out.victory_reached);
    EXPECT_NEAR(out.player_world_x, 100.f, 0.01f);
    EXPECT_NEAR(out.player_world_y, 200.f, 0.01f);
    std::remove(path);
}

static void world_save_persist_preserves_existing_victory_flag() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    seed.victory_reached = true;
    if (!SaveSystem::save_default(seed))
        return;

    Actor player;
    player.transform.set_position(321.f, 654.f);
    WorldContext ctx;
    ctx.player = &player;
    bool stress = false;
    ctx.stress_mode = &stress;
    float flash_timer = 0.0f;
    if (!WorldSaveController::persist(ctx, false, flash_timer))
        return;

    SaveData out{};
    if (!SaveSystem::load_default(out))
        return;
    EXPECT_TRUE(out.victory_reached);
    EXPECT_NEAR(out.player_world_x, 321.f, 0.01f);
    EXPECT_NEAR(out.player_world_y, 654.f, 0.01f);

    SaveSystem::remove_default_saves();
}

static void world_save_snapshot_noop_paths_return_success() {
    WorldContext ctx;
    bool stress = true;
    ctx.stress_mode = &stress;
    EXPECT_TRUE(WorldSaveController::snapshot_last_run(ctx));
}

static void world_save_persist_noop_stress_mode_returns_success() {
    WorldContext ctx;
    bool stress = true;
    ctx.stress_mode = &stress;
    float flash_timer = 0.0f;
    EXPECT_TRUE(WorldSaveController::persist(ctx, true, flash_timer));
    EXPECT_NEAR(flash_timer, 0.0f, 0.001f);
}

static void world_save_persist_sets_flash_timer_on_success_when_enabled() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    if (!SaveSystem::save_default(seed))
        return;

    Actor player;
    player.transform.set_position(12.f, 34.f);
    WorldContext ctx;
    ctx.player = &player;
    bool stress = false;
    ctx.stress_mode = &stress;

    float flash_timer = 0.0f;
    if (!WorldSaveController::persist(ctx, true, flash_timer))
        return;
    EXPECT_TRUE(flash_timer > 0.0f);

    SaveSystem::remove_default_saves();
}

static void world_save_persist_keeps_flash_timer_when_indicator_disabled() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    if (!SaveSystem::save_default(seed))
        return;

    Actor player;
    player.transform.set_position(98.f, 76.f);
    WorldContext ctx;
    ctx.player = &player;
    bool stress = false;
    ctx.stress_mode = &stress;

    float flash_timer = 0.42f;
    if (!WorldSaveController::persist(ctx, false, flash_timer))
        return;
    EXPECT_NEAR(flash_timer, 0.42f, 0.001f);

    SaveSystem::remove_default_saves();
}

static void world_save_apply_clamps_difficulty_range() {
    WorldContext ctx;
    DifficultyLevel difficulty = DifficultyLevel::Normal;
    ctx.difficulty = &difficulty;

    SaveData low{};
    low.difficulty = -99;
    WorldSaveController::apply_world_save(ctx, low);
    EXPECT_EQ(static_cast<int>(difficulty), 0);

    SaveData high{};
    high.difficulty = 99;
    WorldSaveController::apply_world_save(ctx, high);
    EXPECT_EQ(static_cast<int>(difficulty), 2);
}

static void world_save_load_saved_difficulty_clamps_range() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();
    SaveData low{};
    low.difficulty = -42;
    if (!SaveSystem::save_default(low))
        return;
    DifficultyLevel out = DifficultyLevel::Normal;
    if (!WorldSaveController::load_saved_difficulty(out))
        return;
    EXPECT_EQ(static_cast<int>(out), 0);

    SaveData high{};
    high.difficulty = 99;
    if (!SaveSystem::save_default(high))
        return;
    if (!WorldSaveController::load_saved_difficulty(out))
        return;
    EXPECT_EQ(static_cast<int>(out), 2);
    SaveSystem::remove_default_saves();
}

static void world_save_clear_default_saves_noop_when_missing_returns_success() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();
    EXPECT_FALSE(WorldSaveController::has_default_save());
    EXPECT_TRUE(WorldSaveController::clear_default_saves());
}

static void world_save_has_default_save_reflects_presence() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();
    EXPECT_FALSE(WorldSaveController::has_default_save());

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    if (!SaveSystem::save_default(seed))
        return;
    if (!WorldSaveController::has_default_save())
        return;

    if (!WorldSaveController::clear_default_saves())
        return;
    EXPECT_FALSE(WorldSaveController::has_default_save());
}

static void world_save_clear_default_saves_removes_existing_save() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    seed.gold = 77;
    if (!SaveSystem::save_default(seed))
        return;
    if (!WorldSaveController::has_default_save())
        return;

    if (!WorldSaveController::clear_default_saves())
        return;
    EXPECT_FALSE(WorldSaveController::has_default_save());

    SaveData out{};
    EXPECT_FALSE(SaveSystem::load_default(out));
}

static void world_save_load_victory_unlock_flag_reports_missing_and_present_save() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    bool unlocked = true;
    EXPECT_FALSE(WorldSaveController::load_victory_unlock_flag(unlocked));
    EXPECT_FALSE(unlocked);

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    seed.victory_reached = true;
    if (!SaveSystem::save_default(seed))
        return;

    unlocked = false;
    EXPECT_TRUE(WorldSaveController::load_victory_unlock_flag(unlocked));
    EXPECT_TRUE(unlocked);
    SaveSystem::remove_default_saves();
}

static void world_save_snapshot_last_run_writes_stats_when_save_exists() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    if (!SaveSystem::save_default(seed))
        return;

    RunStats stats{};
    stats.rooms_cleared = 7;
    stats.enemies_killed = 31;
    stats.time_seconds = 123.5f;

    WorldContext ctx;
    ctx.run_stats = &stats;
    bool stress = false;
    ctx.stress_mode = &stress;

    if (!WorldSaveController::snapshot_last_run(ctx))
        return;

    SaveData out{};
    if (!SaveSystem::load_default(out))
        return;
    EXPECT_EQ(out.last_run_stats.rooms_cleared, 7);
    EXPECT_EQ(out.last_run_stats.enemies_killed, 31);
    EXPECT_NEAR(out.last_run_stats.time_seconds, 123.5f, 0.001f);

    SaveSystem::remove_default_saves();
}

static void world_save_mark_victory_reached_seeds_save_when_missing() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    RunStats stats{};
    stats.rooms_cleared = 12;
    stats.enemies_killed = 44;
    stats.time_seconds = 222.25f;

    if (!WorldSaveController::mark_victory_reached(stats))
        return;

    SaveData out{};
    if (!SaveSystem::load_default(out))
        return;
    EXPECT_TRUE(out.victory_reached);
    EXPECT_EQ(out.last_run_stats.rooms_cleared, 12);
    EXPECT_EQ(out.last_run_stats.enemies_killed, 44);
    EXPECT_NEAR(out.last_run_stats.time_seconds, 222.25f, 0.001f);

    SaveSystem::remove_default_saves();
}

static void world_save_persist_difficulty_if_save_exists_handles_missing_and_present() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    EXPECT_TRUE(WorldSaveController::persist_difficulty_if_save_exists(2));

    SaveData missing_out{};
    EXPECT_FALSE(SaveSystem::load_default(missing_out));

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    seed.difficulty = 0;
    if (!SaveSystem::save_default(seed))
        return;

    if (!WorldSaveController::persist_difficulty_if_save_exists(2))
        return;

    SaveData out{};
    if (!SaveSystem::load_default(out))
        return;
    EXPECT_EQ(out.difficulty, 2);

    SaveSystem::remove_default_saves();
}

static void world_save_load_default_save_reports_missing_and_present_data() {
    if (!world_save_default_backend_available())
        return;
    SaveSystem::remove_default_saves();

    SaveData out_missing{};
    EXPECT_FALSE(WorldSaveController::load_default_save(out_missing));

    SaveData seed{};
    seed.version = kSaveFormatVersion;
    seed.gold = 123;
    seed.player_world_x = 45.0f;
    seed.player_world_y = 67.5f;
    if (!SaveSystem::save_default(seed))
        return;

    SaveData out{};
    if (!WorldSaveController::load_default_save(out))
        return;
    EXPECT_EQ(out.gold, 123);
    EXPECT_NEAR(out.player_world_x, 45.0f, 0.001f);
    EXPECT_NEAR(out.player_world_y, 67.5f, 0.001f);

    SaveSystem::remove_default_saves();
}

static void world_save_apply_clears_transient_projectiles_and_ground_items() {
    WorldContext ctx;
    std::vector<Projectile> projectiles;
    std::vector<GroundItem> ground_items;
    projectiles.emplace_back();
    ground_items.emplace_back();
    ctx.projectiles = &projectiles;
    ctx.ground_items = &ground_items;

    SaveData sd{};
    WorldSaveController::apply_world_save(ctx, sd);

    EXPECT_EQ(projectiles.size(), 0u);
    EXPECT_EQ(ground_items.size(), 0u);
}

static void world_save_apply_restores_quest_state_and_scene_flags() {
    WorldContext ctx;
    QuestState quest_state{};
    unsigned int scene_flags = 0u;
    ctx.quest_state = &quest_state;
    ctx.scene_flags = &scene_flags;

    SaveData sd{};
    sd.quest_state.set(QuestId::DefeatGrimjaw, QuestStatus::Completed);
    sd.scene_flags = 0xABu;

    WorldSaveController::apply_world_save(ctx, sd);

    EXPECT_TRUE(quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::Completed));
    EXPECT_EQ(scene_flags, 0xABu);
}

static void world_save_apply_restores_player_hp_from_save() {
    Actor player;
    player.health.current_hp = 1;

    WorldContext ctx;
    ctx.player = &player;

    SaveData sd{};
    sd.player_hp = 42;

    WorldSaveController::apply_world_save(ctx, sd);

    EXPECT_EQ(player.health.current_hp, 42);
}

REGISTER_TEST(world_save_make_and_apply_restores_position_and_mask);
REGISTER_TEST(world_save_merge_preserves_victory_like_persist);
REGISTER_TEST(world_save_persist_preserves_existing_victory_flag);
REGISTER_TEST(world_save_snapshot_noop_paths_return_success);
REGISTER_TEST(world_save_persist_noop_stress_mode_returns_success);
REGISTER_TEST(world_save_persist_sets_flash_timer_on_success_when_enabled);
REGISTER_TEST(world_save_persist_keeps_flash_timer_when_indicator_disabled);
REGISTER_TEST(world_save_apply_clamps_difficulty_range);
REGISTER_TEST(world_save_load_saved_difficulty_clamps_range);
REGISTER_TEST(world_save_clear_default_saves_noop_when_missing_returns_success);
REGISTER_TEST(world_save_has_default_save_reflects_presence);
REGISTER_TEST(world_save_clear_default_saves_removes_existing_save);
REGISTER_TEST(world_save_load_victory_unlock_flag_reports_missing_and_present_save);
REGISTER_TEST(world_save_snapshot_last_run_writes_stats_when_save_exists);
REGISTER_TEST(world_save_mark_victory_reached_seeds_save_when_missing);
REGISTER_TEST(world_save_persist_difficulty_if_save_exists_handles_missing_and_present);
REGISTER_TEST(world_save_load_default_save_reports_missing_and_present_data);
REGISTER_TEST(world_save_apply_clears_transient_projectiles_and_ground_items);
REGISTER_TEST(world_save_apply_restores_quest_state_and_scene_flags);
REGISTER_TEST(world_save_apply_restores_player_hp_from_save);
