#include "../test_common.hpp"
#include "components/health.hpp"

static void test_health() {
    mion::HealthState h { 100, 100 };
    EXPECT_TRUE(h.is_alive());

    h.apply_damage(30);
    EXPECT_EQ(h.current_hp, 70);
    EXPECT_TRUE(h.is_alive());

    h.apply_damage(200);
    EXPECT_EQ(h.current_hp, 0);
    EXPECT_FALSE(h.is_alive());

    h.restore_full();
    EXPECT_EQ(h.current_hp, 100);
    EXPECT_TRUE(h.is_alive());
}
REGISTER_TEST(test_health);
