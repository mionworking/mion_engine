#include "../test_common.hpp"

#include "../../src/core/world_layout_ids.hpp"
#include "../../src/systems/town_builder.hpp"

using namespace mion;

static void town_builder_creates_expected_world_layout() {
    RoomDefinition room;
    Tilemap tilemap;
    TilemapRenderer tile_renderer;
    std::vector<NpcEntity> npcs;
    ShopInventory shop;
    Actor player;

    TownBuilder::build_town_world(room, tilemap, tile_renderer, npcs, shop, player, nullptr);

    EXPECT_EQ(room.name, WorldLayoutId::kTown);
    EXPECT_NEAR(room.bounds.max_x, 2400.0f, 0.001f);
    EXPECT_NEAR(room.bounds.max_y, 1600.0f, 0.001f);
    EXPECT_EQ(static_cast<int>(room.obstacles.size()), 4);
    EXPECT_EQ(static_cast<int>(room.doors.size()), 1);
    EXPECT_EQ(room.doors[0].target_scene_id, WorldLayoutId::kDungeon);
    EXPECT_EQ(static_cast<int>(npcs.size()), 4);
    EXPECT_EQ(npcs[0].name, "Mira");
    EXPECT_EQ(npcs[0].type, NpcType::QuestGiver);
    EXPECT_EQ(static_cast<int>(shop.items.size()), 4);
    EXPECT_EQ(shop.items[0].display_name, "HP Potion");
}

REGISTER_TEST(town_builder_creates_expected_world_layout);
