#include "../test_common.hpp"

#include "../../src/systems/area_entry_system.hpp"
#include "../../src/world/world_context.hpp"
#include "../../src/world/world_map.hpp"

using namespace mion;

static WorldMap make_town_and_dungeon() {
    WorldMap map;
    WorldArea town;
    town.zone = ZoneId::Town;
    town.room.bounds = {0.f, 300.f, 0.f, 300.f};
    map.areas.push_back(std::move(town));
    WorldArea d0;
    d0.zone     = ZoneId::DungeonRoom0;
    d0.offset_x = 300.f;
    d0.room.bounds = {0.f, 200.f, 0.f, 200.f};
    map.areas.push_back(std::move(d0));
    return map;
}

static int g_spawn_calls = 0;
static ZoneId g_spawn_zone = ZoneId::Town;

static void track_spawn(WorldArea& area, WorldContext&) {
    ++g_spawn_calls;
    g_spawn_zone = area.zone;
}

static void area_entry_spawns_on_first_dungeon_entry_only() {
    WorldMap map = make_town_and_dungeon();
    Actor player;
    player.transform.set_position(50.f, 50.f);
    ZoneManager zm;
    AreaEntrySystem entry;
    WorldContext ctx;
    ctx.world_map = &map;
    ctx.player    = &player;

    zm.update(player.transform.x, player.transform.y, map);
    entry.update(zm, ctx, track_spawn);
    EXPECT_EQ(g_spawn_calls, 0);

    player.transform.set_position(400.f, 50.f);
    zm.update(player.transform.x, player.transform.y, map);
    EXPECT_TRUE(zm.just_changed);
    g_spawn_calls = 0;
    entry.update(zm, ctx, track_spawn);
    EXPECT_EQ(g_spawn_calls, 1);
    EXPECT_TRUE(g_spawn_zone == ZoneId::DungeonRoom0);

    g_spawn_calls = 0;
    player.transform.set_position(50.f, 50.f);
    zm.update(player.transform.x, player.transform.y, map);
    entry.update(zm, ctx, track_spawn);
    player.transform.set_position(400.f, 50.f);
    zm.update(player.transform.x, player.transform.y, map);
    entry.update(zm, ctx, track_spawn);
    EXPECT_EQ(g_spawn_calls, 0);
}

static void area_entry_mask_roundtrip() {
    AreaEntrySystem entry;
    entry.visited_areas.insert(ZoneId::DungeonRoom0);
    entry.visited_areas.insert(ZoneId::Boss);
    uint8_t m = entry.to_mask();
    EXPECT_EQ(static_cast<int>(m), 0x01 | 0x08);

    AreaEntrySystem e2;
    e2.from_mask(m);
    EXPECT_TRUE(e2.visited_areas.count(ZoneId::DungeonRoom0) != 0);
    EXPECT_TRUE(e2.visited_areas.count(ZoneId::Boss) != 0);
    EXPECT_FALSE(e2.visited_areas.count(ZoneId::DungeonRoom1) != 0);

    e2.from_mask(0);
    EXPECT_TRUE(e2.visited_areas.empty());
}

REGISTER_TEST(area_entry_spawns_on_first_dungeon_entry_only);
REGISTER_TEST(area_entry_mask_roundtrip);
