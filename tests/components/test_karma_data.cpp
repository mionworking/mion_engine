#include "../test_common.hpp"
#include "components/karma_data.hpp"

static void test_karma_default_init() {
    mion::KarmaData k;
    EXPECT_EQ(k.total, 0);
    EXPECT_EQ(k.available, 0);
}
REGISTER_TEST(test_karma_default_init);

static void test_karma_add_positive() {
    mion::KarmaData k;
    mion::karma_add(k, 50);
    EXPECT_EQ(k.total, 50);
    EXPECT_EQ(k.available, 50);

    mion::karma_add(k, 25);
    EXPECT_EQ(k.total, 75);
    EXPECT_EQ(k.available, 75);
}
REGISTER_TEST(test_karma_add_positive);

static void test_karma_add_zero_or_negative_noop() {
    mion::KarmaData k { 100, 100 };
    mion::karma_add(k, 0);
    EXPECT_EQ(k.total, 100);
    EXPECT_EQ(k.available, 100);

    mion::karma_add(k, -50);
    EXPECT_EQ(k.total, 100);
    EXPECT_EQ(k.available, 100);
}
REGISTER_TEST(test_karma_add_zero_or_negative_noop);

static void test_karma_spend_valid() {
    mion::KarmaData k { 100, 100 };
    EXPECT_TRUE(mion::karma_spend(k, 30));
    EXPECT_EQ(k.total, 100);     // total nunca decresce
    EXPECT_EQ(k.available, 70);
}
REGISTER_TEST(test_karma_spend_valid);

static void test_karma_spend_insufficient() {
    mion::KarmaData k { 100, 20 };
    EXPECT_FALSE(mion::karma_spend(k, 50));
    EXPECT_EQ(k.total, 100);
    EXPECT_EQ(k.available, 20);  // nada mudou
}
REGISTER_TEST(test_karma_spend_insufficient);

static void test_karma_spend_zero_or_negative() {
    mion::KarmaData k { 100, 100 };
    EXPECT_FALSE(mion::karma_spend(k, 0));
    EXPECT_FALSE(mion::karma_spend(k, -10));
    EXPECT_EQ(k.total, 100);
    EXPECT_EQ(k.available, 100);
}
REGISTER_TEST(test_karma_spend_zero_or_negative);

static void test_karma_refund() {
    mion::KarmaData k { 100, 40 };
    mion::karma_refund(k, 30);
    EXPECT_EQ(k.total, 100);     // total não muda no refund
    EXPECT_EQ(k.available, 70);

    mion::karma_refund(k, 0);
    mion::karma_refund(k, -5);
    EXPECT_EQ(k.available, 70);  // no-op
}
REGISTER_TEST(test_karma_refund);
