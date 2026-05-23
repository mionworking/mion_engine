#include "../test_common.hpp"
#include "components/attributes.hpp"

namespace {

int attr_sum(const mion::AttributesState& a) {
    return a.vigor + a.forca + a.destreza + a.inteligencia + a.endurance;
}

int count_eq(const mion::AttributesState& a, int value) {
    int n = 0;
    const int vals[] = { a.vigor, a.forca, a.destreza, a.inteligencia, a.endurance };
    for (int v : vals)
        if (v == value) ++n;
    return n;
}

} // namespace

static void test_focus_vigor() {
    mion::AttributesState attrs{};
    mion::AttributeScales sc;
    mion::apply_attribute_focus(attrs, mion::AttributeId::Vigor, sc);
    EXPECT_EQ(attrs.vigor, sc.focus_bonus);
    EXPECT_EQ(attrs.forca, sc.base_gain);
    EXPECT_EQ(attrs.destreza, sc.base_gain);
    EXPECT_EQ(attrs.inteligencia, sc.base_gain);
    EXPECT_EQ(attrs.endurance, sc.base_gain);
}
REGISTER_TEST(test_focus_vigor);

static void test_focus_endurance() {
    mion::AttributesState attrs{};
    mion::AttributeScales sc;
    mion::apply_attribute_focus(attrs, mion::AttributeId::Endurance, sc);
    EXPECT_EQ(attrs.endurance, sc.focus_bonus);
    EXPECT_EQ(attrs.vigor, sc.base_gain);
    EXPECT_EQ(attrs.forca, sc.base_gain);
    EXPECT_EQ(attrs.destreza, sc.base_gain);
    EXPECT_EQ(attrs.inteligencia, sc.base_gain);
}
REGISTER_TEST(test_focus_endurance);

static void test_focus_total_sum() {
    mion::AttributesState attrs{};
    mion::AttributeScales sc;
    mion::apply_attribute_focus(attrs, mion::AttributeId::Forca, sc);
    EXPECT_EQ(attr_sum(attrs), sc.focus_bonus + 4 * sc.base_gain);
}
REGISTER_TEST(test_focus_total_sum);

static void test_focus_stacks_multiple_calls() {
    mion::AttributesState attrs{};
    mion::AttributeScales sc;
    mion::apply_attribute_focus(attrs, mion::AttributeId::Vigor, sc);
    mion::apply_attribute_focus(attrs, mion::AttributeId::Forca, sc);
    mion::apply_attribute_focus(attrs, mion::AttributeId::Destreza, sc);
    const int per_call = sc.focus_bonus + 4 * sc.base_gain;
    EXPECT_EQ(attr_sum(attrs), 3 * per_call);
}
REGISTER_TEST(test_focus_stacks_multiple_calls);

static void test_focus_custom_scales() {
    mion::AttributesState attrs{};
    mion::AttributeScales sc;
    sc.focus_bonus = 5;
    sc.base_gain   = 2;
    mion::apply_attribute_focus(attrs, mion::AttributeId::Inteligencia, sc);
    EXPECT_EQ(attrs.inteligencia, 5);
    EXPECT_EQ(count_eq(attrs, 2), 4);
    EXPECT_EQ(attr_sum(attrs), 5 + 4 * 2);
}
REGISTER_TEST(test_focus_custom_scales);

static void test_focus_all_attribute_ids() {
    mion::AttributeScales sc;
    const mion::AttributeId ids[] = {
        mion::AttributeId::Vigor,
        mion::AttributeId::Forca,
        mion::AttributeId::Destreza,
        mion::AttributeId::Inteligencia,
        mion::AttributeId::Endurance,
    };
    for (mion::AttributeId id : ids) {
        mion::AttributesState attrs{};
        mion::apply_attribute_focus(attrs, id, sc);
        EXPECT_EQ(count_eq(attrs, sc.focus_bonus), 1);
        EXPECT_EQ(count_eq(attrs, sc.base_gain), 4);
        EXPECT_EQ(attr_sum(attrs), sc.focus_bonus + 4 * sc.base_gain);
    }
}
REGISTER_TEST(test_focus_all_attribute_ids);
