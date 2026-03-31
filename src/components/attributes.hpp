#pragma once

#include "equipment.hpp"
#include "progression.hpp"
#include "talent_state.hpp"
#include "talent_tree.hpp"

namespace mion {

// ---------------------------------------------------------------------------
// Atributos base do player (distribuídos via level-up / equipamento futuro).
// Cada ponto de atributo contribui com um bonus constante nos stats derivados.
// ---------------------------------------------------------------------------
struct AttributesState {
    int vigor        = 0;  // +HP max por ponto
    int forca        = 0;  // +dano melee por ponto
    int destreza     = 0;  // +dano ranged por ponto
    int inteligencia = 0;  // +dano spell (%) e +mana max por ponto
    int endurance    = 0;  // +stamina max por ponto
};

// ---------------------------------------------------------------------------
// Escala de cada atributo (data-driven futuramente via attributes.ini).
// ---------------------------------------------------------------------------
struct AttributeScales {
    int   vigor_hp_per_point          = 8;
    int   forca_melee_per_point       = 2;
    int   destreza_ranged_per_point   = 2;
    float intel_spell_pct_per_point   = 0.04f;
    float intel_mana_per_point        = 10.0f;
    float endurance_stamina_per_point = 10.0f;
};

inline AttributeScales g_attribute_scales{};

// ---------------------------------------------------------------------------
// Stats derivados finais do player.
// Pipeline: base (PlayerConfig) + progressão + talentos + atributos + equip.
// Calculados por recompute_player_derived_stats(); lidos pelos sistemas.
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
// Recalcula DerivedStats a partir do estado atual do player.
// Chamar após: level-up, gasto de talento, equip/unequip, inicialização.
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
