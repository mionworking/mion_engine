#include "test_common.hpp"

#include <cstdio>
#include <string>

#include "core/asset_manifest.hpp"
#include "core/bitmap_font.hpp"
#include "core/save_system.hpp"
#include "core/sprite.hpp"
#include "components/attributes.hpp"
#include "components/equipment.hpp"
#include "entities/ground_item.hpp"
#include "entities/npc.hpp"
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

static void test_bitmap_font_text_width_is_consistent() {
    EXPECT_NEAR(mion::text_width("", 2), 0.0f, 0.001f);
    EXPECT_NEAR(mion::text_width("ABC", 1), 27.0f, 0.001f);
    EXPECT_NEAR(mion::text_width("ABC", 2), 54.0f, 0.001f);
}

static void test_draw_sprite_null_texture_is_safe() {
    mion::SpriteFrame f{};
    f.texture = nullptr;
    mion::draw_sprite(nullptr, f, 0.0f, 0.0f, 1.0f, 1.0f, false);
    EXPECT_TRUE(true);
}

static void test_equipment_total_modifiers_accumulate_slots() {
    mion::EquipmentState eq;
    mion::ItemDef weapon;
    weapon.slot = mion::EquipSlot::Weapon;
    weapon.modifiers.melee_damage = 3;
    weapon.modifiers.spell_mult = 0.1f;
    eq.equip(mion::EquipSlot::Weapon, weapon);

    mion::ItemDef armor;
    armor.slot = mion::EquipSlot::Armor;
    armor.modifiers.hp_bonus = 20;
    armor.modifiers.stamina_bonus = 8.0f;
    eq.equip(mion::EquipSlot::Armor, armor);

    mion::ItemModifiers m = eq.total_modifiers();
    EXPECT_EQ(m.melee_damage, 3);
    EXPECT_NEAR(m.spell_mult, 0.1f, 0.001f);
    EXPECT_EQ(m.hp_bonus, 20);
    EXPECT_NEAR(m.stamina_bonus, 8.0f, 0.001f);
}

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
    e.equip(mion::EquipSlot::Accessory, acc);

    mion::recompute_player_derived_stats(d, a, p, t, e, 10, 8);

    EXPECT_EQ(d.melee_damage_final, 18);
    EXPECT_EQ(d.ranged_damage_final, 19);
    EXPECT_NEAR(d.spell_damage_mult, 1.24f, 0.001f);
    EXPECT_EQ(d.hp_max_bonus, 26);
    EXPECT_NEAR(d.stamina_max_bonus, 10.0f, 0.001f);
    EXPECT_NEAR(d.mana_max_bonus, 15.0f, 0.001f);
}

static void test_entities_defaults_for_npc_and_ground_item() {
    mion::NpcEntity npc;
    EXPECT_TRUE(npc.type == mion::NpcType::Generic);
    EXPECT_NEAR(npc.interact_radius, 48.0f, 0.001f);

    mion::GroundItem gi;
    EXPECT_TRUE(gi.type == mion::GroundItemType::Health);
    EXPECT_TRUE(gi.active);
    EXPECT_FALSE(gi.lore_pickup);
}

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

void run_gaps_v2_tests() {
    run("V2.AssetManifest.NoCrash", test_asset_manifest_probe_does_not_crash);
    run("V2.Save.V4AttributesRoundtrip", test_save_system_v4_attributes_roundtrip);
    run("V2.BitmapFont.TextWidth", test_bitmap_font_text_width_is_consistent);
    run("V2.Sprite.NullTextureSafe", test_draw_sprite_null_texture_is_safe);
    run("V2.Equipment.TotalModifiers", test_equipment_total_modifiers_accumulate_slots);
    run("V2.Attributes.RecomputeDerived", test_attributes_recompute_player_derived_stats);
    run("V2.Entities.Defaults", test_entities_defaults_for_npc_and_ground_item);
    run("V2.Render.DefaultConfig", test_render_system_default_configs_are_stable);
}
