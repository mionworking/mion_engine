#pragma once
#include <random>
#include <vector>
#include <algorithm>
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../entities/ground_item.hpp"
#include "../core/ini_loader.hpp"
#include "../core/data_section_names.hpp"

namespace mion {

// Configuração de drops — valores padrão espelham os originais; sobrescrita pelo INI.
struct DropConfig {
    int   drop_chance_pct = 50;    // 0–100
    float pickup_radius   = 36.0f;
    int   health_bonus    = 22;
    int   damage_bonus    = 1;
    float speed_bonus     = 10.0f;
    /// Probabilidade 0–100 de, em drop de saúde, marcar peça de lore (ver items.ini).
    int   lore_drop_chance_pct = 8;
};

struct DropSystem {
    /// `gold_min_override` / `gold_max_override` ≥ 0 substituem o intervalo em `def`.
    static void on_enemy_died(std::vector<GroundItem>& items, float x, float y,
                              const EnemyDef& def, const DropConfig& cfg,
                              std::mt19937& rng,
                              int gold_min_override = -1, int gold_max_override = -1) {
        std::uniform_int_distribution<int> pct(0, 99);
        if (pct(rng) < cfg.drop_chance_pct) {
            GroundItem g;
            g.x = x;
            g.y = y;
            int r = std::uniform_int_distribution<int>(0, 2)(rng);
            g.type = (r == 0) ? GroundItemType::Health
                   : (r == 1) ? GroundItemType::Damage : GroundItemType::Speed;
            g.active = true;
            if (g.type == GroundItemType::Health && cfg.lore_drop_chance_pct > 0
                && pct(rng) < cfg.lore_drop_chance_pct)
                g.lore_pickup = true;
            items.push_back(g);
        }

        int gmin = gold_min_override >= 0 ? gold_min_override : def.gold_drop_min;
        int gmax = gold_max_override >= 0 ? gold_max_override : def.gold_drop_max;
        if (gmax < gmin)
            gmax = gmin;
        const int span   = gmax - gmin + 1;
        const int rolled = gmin + (span > 0 ? std::uniform_int_distribution<int>(0, span - 1)(rng) : 0);
        if (rolled <= 0)
            return;
        GroundItem gold;
        gold.x          = x + 14.0f;
        gold.y          = y - 6.0f;
        gold.type       = GroundItemType::Gold;
        gold.gold_value = rolled;
        gold.active     = true;
        items.push_back(gold);
    }

    /// Retorna true se apanhou um item com `lore_pickup`.
    static bool pickup_near_player(Actor& player, std::vector<GroundItem>& items,
                                   const DropConfig& cfg) {
        bool picked_lore = false;
        const float r2 = cfg.pickup_radius * cfg.pickup_radius;
        for (auto& it : items) {
            if (!it.active) continue;
            float dx = player.transform.x - it.x;
            float dy = player.transform.y - it.y;
            if (dx * dx + dy * dy > r2) continue;

            switch (it.type) {
                case GroundItemType::Health:
                    player.health.current_hp = std::min(
                        player.health.max_hp,
                        player.health.current_hp + cfg.health_bonus);
                    break;
                case GroundItemType::Damage:
                    player.player->progression.bonus_attack_damage += cfg.damage_bonus;
                    break;
                case GroundItemType::Speed:
                    player.player->progression.bonus_move_speed += cfg.speed_bonus;
                    break;
                case GroundItemType::Gold:
                    player.player->gold += it.gold_value;
                    break;
                case GroundItemType::Potion:
                    player.player->potion.pickup(it.potion_quality, it.potion_amount);
                    break;
                case GroundItemType::Equipment:
                    // Bag cheia: item fica no chão (não desativa).
                    if (!it.item_name.empty() && !player.player->bag.is_full()) {
                        player.player->bag.add(it.item_name, it.item_slot);
                    } else {
                        continue; // mantém ativo no chão
                    }
                    break;
            }
            if (it.lore_pickup) picked_lore = true;
            it.active = false;
        }
        items.erase(
            std::remove_if(items.begin(), items.end(),
                           [](const GroundItem& g) { return !g.active; }),
            items.end()
        );
        return picked_lore;
    }
};

inline void apply_drop_ini_overrides(const IniData& d, DropConfig& cfg) {
    cfg.drop_chance_pct =
        d.get_int(data_sections::kDrops, "drop_chance_pct", cfg.drop_chance_pct);
    cfg.pickup_radius =
        d.get_float(data_sections::kDrops, "pickup_radius", cfg.pickup_radius);
    cfg.health_bonus =
        d.get_int(data_sections::kDrops, "health_bonus", cfg.health_bonus);
    cfg.damage_bonus =
        d.get_int(data_sections::kDrops, "damage_bonus", cfg.damage_bonus);
    cfg.speed_bonus =
        d.get_float(data_sections::kDrops, "speed_bonus", cfg.speed_bonus);
    cfg.lore_drop_chance_pct =
        d.get_int(data_sections::kDrops, "lore_drop_chance_pct", cfg.lore_drop_chance_pct);
}

} // namespace mion
