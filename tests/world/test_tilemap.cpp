#include "../test_common.hpp"
#include "world/tilemap.hpp"

static void test_tilemap_init_and_fill() {
    mion::Tilemap tm;
    tm.init(10, 8, 32);
    EXPECT_EQ(tm.cols, 10);
    EXPECT_EQ(tm.rows, 8);
    EXPECT_EQ((int)tm.get(0, 0), (int)mion::TileType::Void);

    tm.fill(0, 0, 9, 7, mion::TileType::Floor);
    EXPECT_EQ(tm.get(5, 3), mion::TileType::Floor);

    tm.set(2, 2, mion::TileType::Wall);
    EXPECT_EQ(tm.get(2, 2), mion::TileType::Wall);

    // Fora dos limites retorna Void
    EXPECT_EQ(tm.get(-1, 0), mion::TileType::Void);
    EXPECT_EQ(tm.get(10, 0), mion::TileType::Void);
}
REGISTER_TEST(test_tilemap_init_and_fill);

static void test_tilemap_world_dimensions() {
    mion::Tilemap tm;
    tm.init(50, 37, 32);
    EXPECT_NEAR(tm.world_width(),  50 * 32.0f, 0.001f);
    EXPECT_NEAR(tm.world_height(), 37 * 32.0f, 0.001f);
}
REGISTER_TEST(test_tilemap_world_dimensions);
