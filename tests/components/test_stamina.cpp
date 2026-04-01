#include "../test_common.hpp"
#include "components/stamina.hpp"

static void test_stamina_consume_clamp_and_regen_delay() {
    mion::StaminaState s;
    s.current = 50.0f;
    s.max = 100.0f;
    s.regen_rate = 40.0f;
    s.regen_delay = 0.4f;
    s.consume(80.0f);
    EXPECT_NEAR(s.current, 0.0f, 0.001f);
    s.tick(0.2f);
    EXPECT_NEAR(s.current, 0.0f, 0.001f);
    s.tick(0.25f);
    s.tick(0.05f);
    EXPECT_TRUE(s.current > 0.0f);
}
REGISTER_TEST(test_stamina_consume_clamp_and_regen_delay);

static void test_stamina_tick_caps_at_max() {
    mion::StaminaState s;
    s.current = 95.0f;
    s.max = 100.0f;
    s.regen_rate = 1000.0f;
    s.regen_delay = 0.0f;
    s.tick(1.0f);
    EXPECT_NEAR(s.current, 100.0f, 0.01f);
}
REGISTER_TEST(test_stamina_tick_caps_at_max);
