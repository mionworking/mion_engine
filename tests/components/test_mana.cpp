#include "../test_common.hpp"
#include "components/mana.hpp"

static void test_mana_consume_and_regen_delay() {
    mion::ManaState m;
    m.max = 100.0f;
    m.current = 100.0f;
    m.regen_rate = 20.0f;
    m.regen_delay = 0.5f;

    m.consume(40.0f);
    EXPECT_NEAR(m.current, 60.0f, 0.001f);

    m.tick(0.25f); // ainda em delay
    EXPECT_NEAR(m.current, 60.0f, 0.001f);

    m.tick(0.35f); // zera delay
    m.tick(0.10f); // agora regenera
    EXPECT_TRUE(m.current > 60.0f);
}
REGISTER_TEST(test_mana_consume_and_regen_delay);

static void test_mana_clamp_not_negative() {
    mion::ManaState m;
    m.current = 10.0f;
    m.consume(25.0f);
    EXPECT_NEAR(m.current, 0.0f, 0.001f);
}
REGISTER_TEST(test_mana_clamp_not_negative);

static void test_mana_regen_to_max() {
    mion::ManaState m;
    m.current = 20.0f;
    m.max = 50.0f;
    m.regen_rate = 100.0f;
    m.regen_delay = 0.0f;

    m.tick(1.0f);
    EXPECT_NEAR(m.current, 50.0f, 0.001f);
}
REGISTER_TEST(test_mana_regen_to_max);
