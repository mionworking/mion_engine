#include "../test_common.hpp"

#include <algorithm>

#include "../../src/core/world_layout_ids.hpp"
#include "../../src/systems/dungeon_config_loader.hpp"
#include "../../src/world/world_map_builder.hpp"

using namespace mion;

static bool aabb_interior_overlap(const AABB& a, const AABB& b) {
    const float ix0 = std::max(a.min_x, b.min_x);
    const float ix1 = std::min(a.max_x, b.max_x);
    const float iy0 = std::max(a.min_y, b.min_y);
    const float iy1 = std::min(a.max_y, b.max_y);
    return ix0 < ix1 - 1e-3f && iy0 < iy1 - 1e-3f;
}

static void world_map_builder_creates_six_non_overlapping_areas_with_nav() {
    EnemyDef enemy_defs[kEnemyTypeCount]{};
    DropConfig drop_config{};
    IniData rooms_ini{};
    std::array<std::string, kEnemyTypeCount> enemy_sprite_paths{};
    DungeonConfigLoader::load_dungeon_static_data(
        enemy_defs, drop_config, rooms_ini, enemy_sprite_paths);

    std::vector<NpcEntity> npcs;
    ShopInventory shop;
    Actor player;
    TilemapRenderer tile_renderer;

    WorldMap map = WorldMapBuilder::build(
        rooms_ini, nullptr, npcs, shop, player, tile_renderer);

    EXPECT_EQ(static_cast<int>(map.areas.size()), 6);

    for (const auto& a : map.areas) {
        if (a.zone == ZoneId::Town) {
            EXPECT_EQ(static_cast<int>(a.room.doors.size()), 1);
            EXPECT_EQ(a.room.doors[0].target_scene_id, WorldLayoutId::kDungeon);
        } else {
            EXPECT_EQ(static_cast<int>(a.room.doors.size()), 0);
        }
    }

    for (size_t i = 0; i < map.areas.size(); ++i) {
        EXPECT_TRUE(map.areas[i].pathfinder.nav.cols > 0);
        EXPECT_TRUE(map.areas[i].pathfinder.nav.rows > 0);
        AABB gi = map.areas[i].global_bounds();
        for (size_t j = i + 1; j < map.areas.size(); ++j) {
            AABB gj = map.areas[j].global_bounds();
            EXPECT_FALSE(aabb_interior_overlap(gi, gj));
        }
    }

    int n_town = 0, n_trans = 0, n_d0 = 0, n_d1 = 0, n_d2 = 0, n_boss = 0;
    for (const auto& a : map.areas) {
        switch (a.zone) {
            case ZoneId::Town: ++n_town; break;
            case ZoneId::Transition: ++n_trans; break;
            case ZoneId::DungeonRoom0: ++n_d0; break;
            case ZoneId::DungeonRoom1: ++n_d1; break;
            case ZoneId::DungeonRoom2: ++n_d2; break;
            case ZoneId::Boss: ++n_boss; break;
        }
    }
    EXPECT_EQ(n_town, 1);
    EXPECT_EQ(n_trans, 1);
    EXPECT_EQ(n_d0, 1);
    EXPECT_EQ(n_d1, 1);
    EXPECT_EQ(n_d2, 1);
    EXPECT_EQ(n_boss, 1);

    const WorldArea* corridor = nullptr;
    for (const auto& a : map.areas) {
        if (a.zone == ZoneId::Transition) {
            corridor = &a;
            break;
        }
    }
    EXPECT_TRUE(corridor != nullptr);
    if (corridor) {
        EXPECT_EQ(corridor->room.name, WorldLayoutId::kCorridor);
        EXPECT_NEAR(corridor->room.bounds.max_y, 900.f, 0.01f);
        AABB gb = corridor->global_bounds();
        EXPECT_NEAR(gb.max_y - gb.min_y, 900.f, 0.01f);
    }
}

REGISTER_TEST(world_map_builder_creates_six_non_overlapping_areas_with_nav);
