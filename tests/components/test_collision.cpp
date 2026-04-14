#include "../test_common.hpp"
#include "components/collision.hpp"

static void test_aabb_intersection() {
    mion::AABB a { 0, 10, 0, 10 };
    mion::AABB b { 5, 15, 5, 15 };
    mion::AABB c { 11, 20, 0, 10 };  // não toca

    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_FALSE(a.intersects(c));
    EXPECT_FALSE(c.intersects(a));
}
REGISTER_TEST(test_aabb_intersection);

static void test_collision_box_bounds() {
    mion::CollisionBox box { 16.0f, 16.0f };
    mion::AABB b = box.bounds_at(100.0f, 100.0f);
    EXPECT_EQ(b.min_x, 84.0f);
    EXPECT_EQ(b.max_x, 116.0f);
    EXPECT_EQ(b.min_y, 84.0f);
    EXPECT_EQ(b.max_y, 116.0f);
}
REGISTER_TEST(test_collision_box_bounds);
