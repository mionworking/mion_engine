#include "../test_common.hpp"
#include "components/combat.hpp"

static void test_combat_phase_transitions() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_TRUE(c.is_attack_idle());

    c.begin_attack();
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Startup);
    EXPECT_FALSE(c.is_in_active_phase());

    // Avança além do startup
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Active);
    EXPECT_TRUE(c.is_in_active_phase());

    // Avança além do active
    c.advance_time(c.attack_active_duration_seconds + 0.01f);
    EXPECT_EQ(c.attack_phase, mion::AttackPhase::Recovery);
    EXPECT_FALSE(c.is_in_active_phase());

    // Avança além do recovery — volta a idle
    c.advance_time(c.attack_recovery_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_attack_idle());
}
REGISTER_TEST(test_combat_phase_transitions);

static void test_combat_hit_landed_reset() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_FALSE(c.attack_hit_landed);

    c.begin_attack();
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_in_active_phase());

    c.attack_hit_landed = true;
    // advance_time faz 1 transição por chamada — avançar em 2 passos: Active→Recovery, Recovery→Idle
    c.advance_time(c.attack_active_duration_seconds + 0.01f);
    c.advance_time(c.attack_recovery_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_attack_idle());
    c.begin_attack();
    EXPECT_FALSE(c.attack_hit_landed);
}
REGISTER_TEST(test_combat_hit_landed_reset);

static void test_combat_hit_reaction_interrupts_attack() {
    mion::CombatState c;
    c.reset_for_spawn();
    c.begin_attack();
    c.advance_time(c.attack_startup_duration_seconds + 0.01f);
    EXPECT_TRUE(c.is_in_active_phase());

    c.apply_hit_reaction();
    EXPECT_TRUE(c.is_attack_idle());
    EXPECT_TRUE(c.is_hurt_stunned());
}
REGISTER_TEST(test_combat_hit_reaction_interrupts_attack);

static void test_combat_invulnerability() {
    mion::CombatState c;
    c.reset_for_spawn();
    EXPECT_TRUE(c.can_receive_hit());

    c.apply_hit_reaction();
    EXPECT_FALSE(c.can_receive_hit());

    // Avança além da invulnerabilidade
    c.advance_time(c.impact_invulnerability_seconds + 0.01f);
    EXPECT_TRUE(c.can_receive_hit());
}
REGISTER_TEST(test_combat_invulnerability);
