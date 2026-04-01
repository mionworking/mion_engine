#include "../test_common.hpp"
#include "core/save_system.hpp"
#include "systems/shop_system.hpp"
#include "core/quest_state.hpp"
#include "entities/actor.hpp"

#include <cstdio>

static void test_save_gold_quest_roundtrip_file() {
    mion::SaveData d;
    d.gold        = 55;
    d.quest_state.set(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::Completed);
    const char* path = "mion_save_town_quest_test.txt";
    std::remove(path);
    EXPECT_TRUE(mion::SaveSystem::save(path, d));
    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.gold, 55);
    EXPECT_TRUE(out.quest_state.is(mion::QuestId::DefeatGrimjaw, mion::QuestStatus::Completed));
    std::remove(path);
}
REGISTER_TEST(test_save_gold_quest_roundtrip_file);

static void test_shop_try_buy_hp_potion() {
    mion::Actor player;
    player.gold                 = 100;
    player.health               = {50, 50};
    mion::ShopInventory shop;
    shop.items.push_back({"HP Potion", mion::ShopItemType::HpPotion, 20, 25});
    shop.selected_index = 0;
    EXPECT_TRUE(mion::ShopSystem::try_buy(player, shop, 0));
    EXPECT_EQ(player.gold, 80);
    EXPECT_EQ(player.health.current_hp, 50);
}
REGISTER_TEST(test_shop_try_buy_hp_potion);

static void test_shop_try_buy_rejects_insufficient_gold() {
    mion::Actor player;
    player.gold = 5;
    mion::ShopInventory shop;
    shop.items.push_back({"HP Potion", mion::ShopItemType::HpPotion, 20, 25});
    EXPECT_FALSE(mion::ShopSystem::try_buy(player, shop, 0));
    EXPECT_EQ(player.gold, 5);
}
REGISTER_TEST(test_shop_try_buy_rejects_insufficient_gold);

static void test_quest_state_int_mapping() {
    EXPECT_EQ(mion::QuestState::status_to_int(mion::QuestStatus::Rewarded), 3);
    EXPECT_TRUE(mion::QuestState::int_to_status(99) == mion::QuestStatus::Rewarded);
}
REGISTER_TEST(test_quest_state_int_mapping);
