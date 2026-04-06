#include "test_common.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "core/asset_manifest.hpp"
#include "core/bitmap_font.hpp"
#include "core/save_system.hpp"
#include "core/sprite.hpp"
#include "components/attributes.hpp"
#include "components/equipment.hpp"
#include "entities/actor.hpp"
#include "entities/ground_item.hpp"
#include "entities/npc.hpp"
#include "systems/melee_combat.hpp"
#include "systems/shop_system.hpp"
#include "systems/lighting.hpp"
#include "systems/tilemap_renderer.hpp"

static bool file_exists(const char* p) {
    std::FILE* f = std::fopen(p, "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

static void test_asset_manifest_probe_does_not_crash() {
    mion::log_missing_assets_optional();
    EXPECT_TRUE(true);
}
REGISTER_TEST(test_asset_manifest_probe_does_not_crash);

static void test_save_system_v4_attributes_roundtrip() {
    const char* path = "mion_save_v4_attr_roundtrip_test.txt";
    std::remove(path);

    mion::SaveData in{};
    in.attributes.vigor = 3;
    in.attributes.forca = 2;
    in.attributes.destreza = 1;
    in.attributes.inteligencia = 4;
    in.attributes.endurance = 5;

    EXPECT_TRUE(mion::SaveSystem::save(path, in));
    EXPECT_TRUE(file_exists(path));

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.attributes.vigor, 3);
    EXPECT_EQ(out.attributes.forca, 2);
    EXPECT_EQ(out.attributes.destreza, 1);
    EXPECT_EQ(out.attributes.inteligencia, 4);
    EXPECT_EQ(out.attributes.endurance, 5);

    std::remove(path);
}
REGISTER_TEST(test_save_system_v4_attributes_roundtrip);

static void test_save_system_v2_chain_migration_keeps_load_valid() {
    const char* path = "mion_save_v2_chain_migration_test.txt";
    std::remove(path);

    {
        std::FILE* f = std::fopen(path, "wb");
        EXPECT_TRUE(f != nullptr);
        if (f) {
            const char* payload =
                "version=2\n"
                "room_index=4\n"
                "hp=88\n"
                "gold=33\n"
                "xp=15\n"
                "level=2\n"
                "xp_to_next=100\n"
                "mana_current=70\n"
                "mana_max=80\n"
                "stamina_current=50\n"
                "stamina_max=60\n";
            std::fwrite(payload, 1, std::strlen(payload), f);
            std::fclose(f);
        }
    }

    mion::SaveData out{};
    EXPECT_TRUE(mion::SaveSystem::load(path, out));
    EXPECT_EQ(out.version, mion::kSaveFormatVersion);
    EXPECT_EQ(out.room_index, 4);
    EXPECT_EQ(out.player_hp, 88);
    EXPECT_EQ(out.gold, 33);
    EXPECT_NEAR(out.attributes.vigor, 0, 0.001f);

    std::remove(path);
}
REGISTER_TEST(test_save_system_v2_chain_migration_keeps_load_valid);

static void test_bitmap_font_text_width_is_consistent() {
    EXPECT_NEAR(mion::text_width("", 2), 0.0f, 0.001f);
    EXPECT_NEAR(mion::text_width("ABC", 1), 27.0f, 0.001f);
    EXPECT_NEAR(mion::text_width("ABC", 2), 54.0f, 0.001f);
}
REGISTER_TEST(test_bitmap_font_text_width_is_consistent);

static void test_draw_sprite_null_texture_is_safe() {
    mion::SpriteFrame f{};
    f.texture = nullptr;
    mion::draw_sprite(nullptr, f, 0.0f, 0.0f, 1.0f, 1.0f, false);
    EXPECT_TRUE(true);
}
REGISTER_TEST(test_draw_sprite_null_texture_is_safe);

static void test_equipment_total_modifiers_accumulate_slots() {
    mion::EquipmentState eq;
    mion::ItemDef weapon;
    weapon.slot = mion::EquipSlot::MainHand;
    weapon.modifiers.melee_damage = 3;
    weapon.modifiers.spell_mult = 0.1f;
    eq.equip(mion::EquipSlot::MainHand, weapon);

    mion::ItemDef armor;
    armor.slot = mion::EquipSlot::Chest;
    armor.modifiers.hp_bonus = 20;
    armor.modifiers.stamina_bonus = 8.0f;
    eq.equip(mion::EquipSlot::Chest, armor);

    mion::ItemModifiers m = eq.total_modifiers();
    EXPECT_EQ(m.melee_damage, 3);
    EXPECT_NEAR(m.spell_mult, 0.1f, 0.001f);
    EXPECT_EQ(m.hp_bonus, 20);
    EXPECT_NEAR(m.stamina_bonus, 8.0f, 0.001f);
}
REGISTER_TEST(test_equipment_total_modifiers_accumulate_slots);

static void test_attributes_recompute_player_derived_stats() {
    mion::DerivedStats d{};
    mion::AttributesState a{};
    a.forca = 2;
    a.destreza = 3;
    a.inteligencia = 1;
    a.vigor = 2;
    a.endurance = 1;
    mion::ProgressionState p{};
    p.bonus_attack_damage = 4;
    p.bonus_max_hp = 10;
    p.spell_damage_multiplier = 1.2f;
    mion::TalentState t{};
    t.levels[static_cast<int>(mion::TalentId::SharpArrow)] = 2;
    mion::EquipmentState e{};
    mion::ItemDef acc;
    acc.modifiers.ranged_damage = 1;
    acc.modifiers.mana_bonus = 5.0f;
    e.equip(mion::EquipSlot::Amulet, acc);

    mion::recompute_player_derived_stats(d, a, p, t, e, 10, 8);

    EXPECT_EQ(d.melee_damage_final, 18);
    EXPECT_EQ(d.ranged_damage_final, 19);
    EXPECT_NEAR(d.spell_damage_mult, 1.24f, 0.001f);
    EXPECT_EQ(d.hp_max_bonus, 26);
    EXPECT_NEAR(d.stamina_max_bonus, 10.0f, 0.001f);
    EXPECT_NEAR(d.mana_max_bonus, 15.0f, 0.001f);
}
REGISTER_TEST(test_attributes_recompute_player_derived_stats);

static void test_shop_attack_upgrade_emits_runtime_signal() {
    mion::Actor player;
    player.gold = 200;
    player.attack_damage = 10;
    player.ranged_damage = 8;
    player.derived.melee_damage_final = 10;

    mion::ShopInventory shop;
    shop.items.push_back({"Sharpening Stone", mion::ShopItemType::AttackUpgrade, 30, 3});
    shop.selected_index = 0;

    EXPECT_TRUE(mion::ShopSystem::try_buy(player, shop, 0));
    EXPECT_EQ(player.progression.bonus_attack_damage, 3);
    EXPECT_EQ(player.derived.melee_damage_final, 13);
}
REGISTER_TEST(test_shop_attack_upgrade_emits_runtime_signal);

static void test_enemy_melee_uses_enemy_damage_path_runtime_signal() {
    mion::Actor enemy;
    enemy.team = mion::Team::Enemy;
    enemy.is_alive = true;
    enemy.attack_damage = 37;
    enemy.derived.melee_damage_final = 11;
    enemy.transform.set_position(0.0f, 0.0f);
    enemy.facing_x = 1.0f;
    enemy.facing_y = 0.0f;
    enemy.combat.begin_attack();
    enemy.combat.advance_time(enemy.combat.attack_startup_duration_seconds + 0.001f);

    mion::Actor player;
    player.team = mion::Team::Player;
    player.is_alive = true;
    player.health.current_hp = 100;
    player.health.max_hp = 100;
    player.transform.set_position(10.0f, 0.0f);

    std::vector<mion::Actor*> actors{&enemy, &player};
    mion::MeleeCombatSystem combat;
    combat.fixed_update(actors, 1.0f / 60.0f);
    EXPECT_EQ(player.health.current_hp, 89);
}
REGISTER_TEST(test_enemy_melee_uses_enemy_damage_path_runtime_signal);

static void test_entities_defaults_for_npc_and_ground_item() {
    mion::NpcEntity npc;
    EXPECT_TRUE(npc.type == mion::NpcType::Generic);
    EXPECT_NEAR(npc.interact_radius, 48.0f, 0.001f);

    mion::GroundItem gi;
    EXPECT_TRUE(gi.type == mion::GroundItemType::Health);
    EXPECT_TRUE(gi.active);
    EXPECT_FALSE(gi.lore_pickup);
}
REGISTER_TEST(test_entities_defaults_for_npc_and_ground_item);

static void test_render_system_default_configs_are_stable() {
    mion::LightingSystem light;
    EXPECT_NEAR(light.torch_radius, 220.0f, 0.001f);
    EXPECT_NEAR(light.torch_softness, 120.0f, 0.001f);
    EXPECT_EQ((int)light.darkness, 210);

    mion::TilemapRenderer tr;
    EXPECT_EQ(tr.floor_tile_col, 0);
    EXPECT_EQ(tr.wall_tile_col, 1);
    EXPECT_EQ((int)mion::TILE_COLOR[(int)mion::TileType::Floor].r, 38);
}
REGISTER_TEST(test_render_system_default_configs_are_stable);
