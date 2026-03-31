#pragma once
#include <algorithm>
#include <SDL3/SDL.h>

#include "../entities/actor.hpp"
#include "../entities/shop.hpp"
#include "../core/bitmap_font.hpp"

namespace mion {

struct ShopSystem {
    static bool try_buy(Actor& player, ShopInventory& shop, int item_index) {
        if (item_index < 0 || item_index >= static_cast<int>(shop.items.size()))
            return false;
        const ShopItem& it = shop.items[static_cast<size_t>(item_index)];
        if (player.gold < it.gold_cost)
            return false;
        player.gold -= it.gold_cost;
        switch (it.type) {
        case ShopItemType::HpPotion:
            player.health.current_hp =
                std::min(player.health.max_hp,
                         player.health.current_hp + it.value);
            break;
        case ShopItemType::StaminaPotion:
            player.stamina.current =
                std::min(player.stamina.max,
                         player.stamina.current + (float)it.value);
            break;
        case ShopItemType::AttackUpgrade:
            player.progression.bonus_attack_damage += it.value;
            break;
        case ShopItemType::ManaUpgrade:
            player.mana.max += (float)it.value;
            player.mana.current += (float)it.value;
            if (player.mana.current > player.mana.max)
                player.mana.current = player.mana.max;
            break;
        }
        recompute_player_derived_stats(
            player.derived,
            player.attributes,
            player.progression,
            player.talents,
            player.equipment,
            player.attack_damage,
            player.ranged_damage);
        return true;
    }

    static void render_shop_ui(SDL_Renderer* r, const ShopInventory& shop,
                               int player_gold, int viewport_w, int viewport_h) {
        const float panel_w = 320.0f;
        const float pad     = 16.0f;
        const float x0      = (float)viewport_w - panel_w - pad;
        const float y0      = pad;
        const float h       = (float)viewport_h - pad * 2.0f;

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 20, 18, 32, 230);
        SDL_FRect bg{x0, y0, panel_w, h};
        SDL_RenderFillRect(r, &bg);
        SDL_SetRenderDrawColor(r, 140, 120, 200, 255);
        SDL_RenderRect(r, &bg);

        const int scale = 2;
        float       ty  = y0 + pad;
        draw_text(r, x0 + pad, ty, "FORGE", scale, 220, 200, 140, 255);
        ty += 28.0f;

        char gold_line[64];
        std::snprintf(gold_line, sizeof(gold_line), "Gold: %d", player_gold);
        draw_text(r, x0 + pad, ty, gold_line, scale, 255, 220, 100, 255);
        ty += 36.0f;

        for (int i = 0; i < static_cast<int>(shop.items.size()); ++i) {
            const ShopItem& it = shop.items[static_cast<size_t>(i)];
            bool              sel = (i == shop.selected_index);
            if (sel) {
                SDL_SetRenderDrawColor(r, 60, 50, 90, 255);
                SDL_FRect row{x0 + 8.0f, ty - 2.0f, panel_w - 16.0f, 22.0f};
                SDL_RenderFillRect(r, &row);
            }
            char line[128];
            std::snprintf(line, sizeof(line), "%s  %dg", it.display_name.c_str(),
                          it.gold_cost);
            draw_text(r, x0 + pad, ty, line, scale,
                      sel ? 255 : 200, sel ? 255 : 200, sel ? 220 : 200, 255);
            ty += 24.0f;
        }

        ty = y0 + h - pad - 48.0f;
        draw_text(r, x0 + pad, ty, "ENTER - comprar", scale, 160, 200, 160, 255);
        ty += 22.0f;
        draw_text(r, x0 + pad, ty, "BACKSPACE - fechar", scale, 140, 140, 160, 255);
    }
};

} // namespace mion
