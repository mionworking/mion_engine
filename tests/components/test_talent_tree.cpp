#include "../test_common.hpp"
#include "components/talent_tree.hpp"
#include "components/talent_state.hpp"
#include "core/ini_loader.hpp"

static void test_talents_ini_display_name() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["display_name"] = "Fluxo de Mana";
    mion::apply_talents_ini(d);
    EXPECT_TRUE(mion::talent_display_name(mion::TalentId::ManaFlow) == "Fluxo de Mana");
    mion::reset_talent_tree_defaults();
}
REGISTER_TEST(test_talents_ini_display_name);

static void test_talent_ini_cost_per_level_affects_unlock() {
    mion::reset_talent_tree_defaults();
    mion::IniData d;
    d.sections["mana_flow"]["cost_per_level"] = "3";
    mion::apply_talents_ini(d);

    mion::TalentState t{};
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ArcaneReservoir));
    EXPECT_EQ(t.pending_points, 2);
    EXPECT_FALSE(t.can_unlock(mion::TalentId::ManaFlow));
    t.pending_points = 3;
    EXPECT_TRUE(t.try_unlock(mion::TalentId::ManaFlow));

    mion::reset_talent_tree_defaults();
}
REGISTER_TEST(test_talent_ini_cost_per_level_affects_unlock);
