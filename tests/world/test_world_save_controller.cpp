#include "../test_common.hpp"

#include <cstdio>

#include "../../src/core/save_system.hpp"
#include "../../src/systems/area_entry_system.hpp"
#include "../../src/systems/world_save_controller.hpp"
#include "../../src/world/world_context.hpp"
#include "../../src/world/world_map.hpp"

using namespace mion;

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

REGISTER_TEST(world_save_make_and_apply_restores_position_and_mask);
REGISTER_TEST(world_save_merge_preserves_victory_like_persist);
