#pragma once
#include <algorithm>
#include <SDL3/SDL.h>

#include "../core/bitmap_font.hpp"
#include "../core/ui.hpp"
#include "../entities/actor.hpp"
#include "../components/spell_book.hpp"
#include "../components/spell_defs.hpp"
#include "../components/status_effect.hpp"

namespace mion {

inline void render_dungeon_hud(SDL_Renderer* r, int vw, int vh,
                                const Actor& player, int room_index)
{
    if (!player.is_alive) return;

    const float x = 20.0f;
    const float y_hp  = (float)vh - 28.0f;
    const float y_st  = y_hp - 18.0f;
    const float y_mp  = y_st - 18.0f;
    const float y_xp  = 18.0f;
    const float w = 160.0f, h = 10.0f;

    // XP / level
    float xp_ratio = (player.progression.xp_to_next > 0)
        ? (float)player.progression.xp / player.progression.xp_to_next : 0.0f;
    SDL_SetRenderDrawColor(r, 30, 30, 50, 220);
    SDL_FRect xp_bg = { x, y_xp, w, h };
    SDL_RenderFillRect(r, &xp_bg);
    SDL_SetRenderDrawColor(r, 120, 180, 255, 255);
    SDL_FRect xp_fill = { x, y_xp, w * xp_ratio, h };
    SDL_RenderFillRect(r, &xp_fill);
    char xb[48];
    SDL_snprintf(xb, sizeof(xb), "ROOM %d", room_index + 1);
    draw_text(r, x, 2.0f, xb, 1, 200, 200, 220);
    char lb[16];
    SDL_snprintf(lb, sizeof(lb), "Lv%d", player.progression.level);
    draw_text(r, 8.0f, 48.0f, lb, 2, 200, 200, 180, 255);
    char gb[32];
    SDL_snprintf(gb, sizeof(gb), "G %d", player.gold);
    draw_text(r, (float)vw - text_width(gb, 2) - 12.0f, 12.0f, gb, 2, 255, 215, 0, 255);
    if (player.talents.pending_points > 0) {
        char tb[24];
        SDL_snprintf(tb, sizeof(tb), "TP %d", player.talents.pending_points);
        draw_text(r, x + 120.0f, 2.0f, tb, 1, 150, 220, 255);
    }
    draw_text(r, x + w + 5.0f, y_xp + 1.0f, "XP", 1, 150, 180, 230);

    // Stamina
    float st_ratio = (player.stamina.max > 0.0f)
        ? player.stamina.current / player.stamina.max : 0.0f;
    SDL_SetRenderDrawColor(r, 20, 40, 25, 220);
    SDL_FRect st_bg = { x, y_st, w, h };
    SDL_RenderFillRect(r, &st_bg);
    SDL_SetRenderDrawColor(r, 80, 220, 120, 255);
    SDL_FRect st_fill = { x, y_st, w * st_ratio, h };
    SDL_RenderFillRect(r, &st_fill);
    draw_text(r, x + w + 5.0f, y_st + 1.0f, "ST", 1, 130, 210, 150);

    // Mana
    float mp_ratio = (player.mana.max > 0.0f)
        ? player.mana.current / player.mana.max : 0.0f;
    SDL_SetRenderDrawColor(r, 15, 25, 55, 220);
    SDL_FRect mp_bg = { x, y_mp, w, h };
    SDL_RenderFillRect(r, &mp_bg);
    SDL_SetRenderDrawColor(r, 90, 150, 255, 255);
    SDL_FRect mp_fill = { x, y_mp, w * mp_ratio, h };
    SDL_RenderFillRect(r, &mp_fill);
    draw_text(r, x + w + 5.0f, y_mp + 1.0f, "MP", 1, 120, 170, 240);

    // HP
    float hp_ratio = (player.health.max_hp > 0)
        ? (float)player.health.current_hp / player.health.max_hp : 0.0f;
    SDL_SetRenderDrawColor(r, 40, 10, 10, 220);
    SDL_FRect bg = { x, y_hp, w, h };
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r,
        (Uint8)(255*(1.0f-hp_ratio)), (Uint8)(255*hp_ratio), 30, 255);
    SDL_FRect fill = { x, y_hp, w * hp_ratio, h };
    SDL_RenderFillRect(r, &fill);
    char buf[28];
    SDL_snprintf(buf, sizeof(buf), "HP %d/%d",
                 player.health.current_hp, player.health.max_hp);
    draw_text(r, x, y_hp - 14.0f, buf, 1, 200, 200, 200);

    float ystat = y_hp - 42.0f;
    if (player.status_effects.has(StatusEffectType::Poison)) {
        draw_text(r, x, ystat, "[POISON]", 1, 150, 230, 130);
        ystat -= 12.0f;
    }
    if (player.status_effects.has(StatusEffectType::Slow)) {
        draw_text(r, x, ystat, "[SLOW]", 1, 120, 200, 255);
        ystat -= 12.0f;
    }
    if (player.status_effects.has(StatusEffectType::Stun)) {
        draw_text(r, x, ystat, "[STUN]", 1, 255, 220, 100);
    }

    // XP footer bar
    {
        float xp_r = (player.progression.xp_to_next > 0)
            ? (float)player.progression.xp / (float)player.progression.xp_to_next : 0.0f;
        ui::ProgressBar xp_footer;
        xp_footer.rect       = {0.0f, (float)vh - 6.0f, (float)vw, 6.0f};
        xp_footer.value      = xp_r;
        xp_footer.color_fill = {80, 160, 255, 255};
        xp_footer.color_bg   = {25, 30, 45, 220};
        xp_footer.render(r);
    }

    // Hotbar
    const float hot_y  = (float)vh - 52.0f;
    const float slot_w = 64.0f;
    const float cx     = (float)vw * 0.5f;
    auto draw_slot = [&](int idx, const char* key, SpellId sid, const char* abbrev) {
        const float sx   = cx - slot_w * 2.0f - 24.0f + (slot_w + 10.0f) * (float)idx;
        const int   si   = static_cast<int>(sid);
        const bool  unlk = player.spell_book.is_unlocked(sid);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 20, 18, 35, 230);
        SDL_FRect box{sx, hot_y, slot_w, 40.0f};
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 120, 100, 80, 255);
        SDL_RenderRect(r, &box);
        draw_text(r, sx + 4.0f, hot_y + 2.0f, key, 1, 160, 160, 200, 255);
        const char* tag = unlk ? abbrev : "--";
        draw_text(r, sx + 4.0f, hot_y + 16.0f, tag, 1, unlk ? 200 : 90, unlk ? 220 : 80,
                  unlk ? 200 : 80, 255);
        if (unlk && player.spell_book.cooldown_remaining[si] > 0.0f) {
            const SpellDef& def = spell_def(sid);
            const float     cd  = def.effective_cooldown(
                spell_damage_rank(sid, player.talents));
            const float t  = player.spell_book.cooldown_remaining[si];
            const float p  = cd > 0.001f ? std::clamp(t / cd, 0.0f, 1.0f) : 1.0f;
            SDL_SetRenderDrawColor(r, 0, 0, 0, 140);
            SDL_FRect dim{sx, hot_y, slot_w, 40.0f * p};
            SDL_RenderFillRect(r, &dim);
        }
    };
    draw_slot(0, "Q", SpellId::FrostBolt, "Ice");
    draw_slot(1, "E", SpellId::Nova, "Nova");
    {
        SpellId rid = SpellId::ChainLightning;
        const char* lab = "Chn";
        if (!player.spell_book.is_unlocked(SpellId::ChainLightning)
            && player.spell_book.is_unlocked(SpellId::Strafe)) {
            rid = SpellId::Strafe;
            lab = "Str";
        } else if (!player.spell_book.is_unlocked(SpellId::ChainLightning)) {
            lab = "--";
        }
        draw_slot(2, "R", rid, lab);
    }
    draw_slot(3, "F", SpellId::BattleCry, "Cry");
}

} // namespace mion
