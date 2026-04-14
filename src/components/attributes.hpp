#pragma once

#include "../core/ini_loader.hpp"
#include "equipment.hpp"
#include "progression.hpp"
#include "talent_state.hpp"
#include "talent_tree.hpp"

namespace mion {

// ---------------------------------------------------------------------------
// Player base attributes (distributed via level-up / future equipment).
// Each attribute point contributes a constant bonus to the derived stats.
// ---------------------------------------------------------------------------
struct AttributesState {
    int vigor        = 0;  // +max HP per point
    int forca        = 0;  // +melee damage per point
    int destreza     = 0;  // +ranged damage per point
    int inteligencia = 0;  // +spell damage (%) and +max mana per point
    int endurance    = 0;  // +max stamina per point
};

// ---------------------------------------------------------------------------
// Per-attribute scaling constants (data/attributes.ini).
// ---------------------------------------------------------------------------
struct AttributeScales {
    int   vigor_hp_per_point          = 8;
    int   forca_melee_per_point       = 2;
    int   destreza_ranged_per_point   = 2;
    float intel_spell_pct_per_point   = 0.04f;
    float intel_mana_per_point        = 10.0f;
    float endurance_stamina_per_point = 10.0f;
};

inline constexpr AttributeScales kDefaultAttributeScales{};

// Fábrica pura — não toca nenhum global. Chaves ausentes mantêm base.
inline AttributeScales make_attribute_scales_from_ini(const IniData& d,
                                                       const AttributeScales& base = kDefaultAttributeScales) {
    AttributeScales sc = base;
    const std::string sec = "attributes";
    sc.vigor_hp_per_point          = d.get_int(sec,   "vigor_hp_per_point",          sc.vigor_hp_per_point);
    sc.forca_melee_per_point       = d.get_int(sec,   "forca_melee_per_point",       sc.forca_melee_per_point);
    sc.destreza_ranged_per_point   = d.get_int(sec,   "destreza_ranged_per_point",   sc.destreza_ranged_per_point);
    sc.intel_spell_pct_per_point   = d.get_float(sec, "intel_spell_pct_per_point",   sc.intel_spell_pct_per_point);
    sc.intel_mana_per_point        = d.get_float(sec, "intel_mana_per_point",        sc.intel_mana_per_point);
    sc.endurance_stamina_per_point = d.get_float(sec, "endurance_stamina_per_point", sc.endurance_stamina_per_point);
    return sc;
}

// Global config — read-only após bootstrap. Escrita apenas em init (make_attribute_scales_from_ini).
namespace detail { inline AttributeScales _g_attribute_scales_mutable = kDefaultAttributeScales; }
inline const AttributeScales& g_attribute_scales = detail::_g_attribute_scales_mutable;

inline void reset_attribute_scales_defaults() { detail::_g_attribute_scales_mutable = kDefaultAttributeScales; }

// ---------------------------------------------------------------------------
// Final derived stats for the player.
// Pipeline: base (PlayerConfig) + progression + talents + attributes + equip.
// Computed by recompute_player_derived_stats(); read by all systems.
// ---------------------------------------------------------------------------
struct DerivedStats {
    int   melee_damage_final  = 10;
    int   ranged_damage_final = 8;
    float spell_damage_mult   = 1.0f;
    int   hp_max_bonus        = 0;
    float stamina_max_bonus   = 0.0f;
    float mana_max_bonus      = 0.0f;
};

// ---------------------------------------------------------------------------
// Recomputes DerivedStats from the current player state.
// Call after: level-up, talent spend, equip/unequip, initialization.
// ---------------------------------------------------------------------------
inline void recompute_player_derived_stats(
    DerivedStats&           derived,
    const AttributesState&  attrs,
    const ProgressionState& prog,
    const TalentState&      talents,
    const EquipmentState&   equip,
    int                     base_melee,
    int                     base_ranged)
{
    const AttributeScales& sc  = g_attribute_scales;
    const ItemModifiers    mod = equip.total_modifiers();

    derived.melee_damage_final =
        base_melee
        + prog.bonus_attack_damage
        + attrs.forca * sc.forca_melee_per_point
        + mod.melee_damage;

    derived.ranged_damage_final =
        base_ranged
        + prog.bonus_attack_damage / 2
        + attrs.destreza * sc.destreza_ranged_per_point
        + talents.level_of(TalentId::SharpArrow)
        + mod.ranged_damage;

    derived.spell_damage_mult =
        prog.spell_damage_multiplier
        + (float)attrs.inteligencia * sc.intel_spell_pct_per_point
        + mod.spell_mult;

    derived.hp_max_bonus =
        prog.bonus_max_hp
        + attrs.vigor * sc.vigor_hp_per_point
        + mod.hp_bonus;

    derived.stamina_max_bonus =
        (float)attrs.endurance * sc.endurance_stamina_per_point
        + mod.stamina_bonus;

    derived.mana_max_bonus =
        (float)attrs.inteligencia * sc.intel_mana_per_point
        + mod.mana_bonus;
}

} // namespace mion
