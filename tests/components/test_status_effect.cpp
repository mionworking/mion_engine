#include "../test_common.hpp"
#include "components/status_effect.hpp"

static void test_status_effect_has_and_apply_replace() {
    mion::StatusEffectState st;
    EXPECT_FALSE(st.has(mion::StatusEffectType::Poison));
    st.apply(mion::StatusEffectType::Poison, 2.0f);
    EXPECT_TRUE(st.has(mion::StatusEffectType::Poison));
    st.apply(mion::StatusEffectType::Poison, 5.0f);
    EXPECT_TRUE(st.slots[0].remaining > 4.0f);
}
REGISTER_TEST(test_status_effect_has_and_apply_replace);

static void test_status_effect_slow_multiplier_min() {
    mion::StatusEffectState st;
    st.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.7f);
    st.apply_full(mion::StatusEffectType::Slow, 1.0f, 0.5f, 0, 0.4f);
    EXPECT_NEAR(st.slow_multiplier(), 0.4f, 0.001f);
}
REGISTER_TEST(test_status_effect_slow_multiplier_min);

static void test_status_effect_clear_expired() {
    mion::StatusEffectState st;
    st.apply(mion::StatusEffectType::Stun, 0.05f);
    st.slots[0].remaining = -0.01f;
    st.clear_expired();
    EXPECT_FALSE(st.has(mion::StatusEffectType::Stun));
}
REGISTER_TEST(test_status_effect_clear_expired);

static void test_status_effect_full_slots_overwrites_slot0() {
    mion::StatusEffectState st;
    st.apply(mion::StatusEffectType::Poison, 1.0f);
    st.apply(mion::StatusEffectType::Slow, 1.0f);
    st.apply(mion::StatusEffectType::Stun, 1.0f);
    st.apply_full(mion::StatusEffectType::Poison, 9.0f, 0.1f, 1, 0.5f);
    EXPECT_TRUE(st.slots[0].type == mion::StatusEffectType::Poison);
    EXPECT_NEAR(st.slots[0].remaining, 9.0f, 0.001f);
}
REGISTER_TEST(test_status_effect_full_slots_overwrites_slot0);
