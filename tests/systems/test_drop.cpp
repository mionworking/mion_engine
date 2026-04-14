#include "../test_common.hpp"
#include <algorithm>
#include <random>
#include <vector>
#include "entities/actor.hpp"
#include "entities/enemy_type.hpp"
#include "systems/drop_system.hpp"

static void test_drop_config_defaults() {
    mion::DropConfig cfg;
    EXPECT_EQ(cfg.drop_chance_pct, 50);
    EXPECT_NEAR(cfg.pickup_radius, 36.0f, 0.01f);
    EXPECT_EQ(cfg.health_bonus, 22);
    EXPECT_EQ(cfg.damage_bonus, 1);
    EXPECT_NEAR(cfg.speed_bonus, 10.0f, 0.01f);
    EXPECT_EQ(cfg.lore_drop_chance_pct, 8);
}
REGISTER_TEST(test_drop_config_defaults);

static void test_drop_system_full_chance_always_drops() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 100;
    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& def = mion::get_enemy_def(mion::EnemyType::Skeleton);
    std::mt19937 rng(12345);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, def, cfg, rng);
    EXPECT_TRUE((int)items.size() >= 2);
    EXPECT_TRUE(items[0].active);
}
REGISTER_TEST(test_drop_system_full_chance_always_drops);

static void test_drop_system_zero_chance_still_drops_gold() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 0;
    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& def = mion::get_enemy_def(mion::EnemyType::Orc);
    std::mt19937 rng(12345);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, def, cfg, rng);
    EXPECT_EQ((int)items.size(), 1);
    if (!items.empty()) {
        EXPECT_EQ(items[0].type, mion::GroundItemType::Gold);
        EXPECT_GT(items[0].gold_value, 0);
    }
}
REGISTER_TEST(test_drop_system_zero_chance_still_drops_gold);

static void test_drop_system_pickup_heals_with_config_bonus() {
    mion::DropConfig cfg;
    cfg.drop_chance_pct = 100;
    cfg.pickup_radius   = 50.0f;
    cfg.health_bonus    = 40;

    std::vector<mion::GroundItem> items;
    const mion::EnemyDef& sk = mion::get_enemy_def(mion::EnemyType::Skeleton);
    std::mt19937 rng(12345);
    mion::DropSystem::on_enemy_died(items, 0.0f, 0.0f, sk, cfg, rng);
    items.erase(std::remove_if(items.begin(), items.end(),
                               [](const mion::GroundItem& g) {
                                   return g.type == mion::GroundItemType::Gold;
                               }),
                items.end());
    for (auto& it : items) it.type = mion::GroundItemType::Health;

    mion::Actor player;
    player.health = { 100, 60 };  // max_hp=100, current_hp=60
    player.transform.set_position(0.0f, 0.0f);

    EXPECT_FALSE(mion::DropSystem::pickup_near_player(player, items, cfg));
    EXPECT_EQ(player.health.current_hp, 100); // 60 + 40 = 100 (clamped ao max)
    EXPECT_TRUE(items.empty());               // item consumido e removido
}
REGISTER_TEST(test_drop_system_pickup_heals_with_config_bonus);

static void test_drop_lore_pickup_returns_true() {
    mion::DropConfig cfg;
    cfg.pickup_radius = 50.0f;
    cfg.health_bonus  = 10;

    std::vector<mion::GroundItem> items;
    mion::GroundItem g;
    g.x           = 0.0f;
    g.y           = 0.0f;
    g.type        = mion::GroundItemType::Health;
    g.lore_pickup = true;
    g.active      = true;
    items.push_back(g);

    mion::Actor player;
    player.health = { 100, 50 };
    player.transform.set_position(0.0f, 0.0f);

    EXPECT_TRUE(mion::DropSystem::pickup_near_player(player, items, cfg));
    EXPECT_EQ(player.health.current_hp, 60);
    EXPECT_TRUE(items.empty());
}
REGISTER_TEST(test_drop_lore_pickup_returns_true);
