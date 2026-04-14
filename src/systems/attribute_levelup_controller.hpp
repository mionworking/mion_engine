#pragma once

#include <SDL3/SDL.h>

#include "../core/locale.hpp"
#include "../components/progression.hpp"
#include "../components/attributes.hpp"
#include "../components/player_config.hpp"
#include "../entities/actor.hpp"
#include "overlay_input.hpp"
#include "attribute_screen_ui.hpp"
#include "ui_controller_contract.hpp"

namespace mion {

struct AttributeLevelUpResult : UiControllerResult {
    bool screen_open   = false;
    bool just_opened   = false;
    bool just_closed   = false;
    int  applied_index = -1;
};

class AttributeLevelUpController {
public:
    void set_locale(LocaleSystem* locale) {
        _locale = locale;
    }

    void sync_open_from_progression(const Actor& player) {
        _just_opened = false;
        _just_closed = false;

        if (!player.player->progression.level_choice_pending()) {
            if (_screen_open) {
                _screen_open = false;
                _just_closed = true;
            }
            return;
        }

        if (!_screen_open) {
            _screen_open    = true;
            _selected_index = 0;
            _just_opened    = true;
        }
    }

    AttributeLevelUpResult update(Actor& player,
                                  const OverlayInputEdges& input,
                                  bool stress_mode)
    {
        AttributeLevelUpResult out{};
        out.screen_open = _screen_open;
        out.just_opened = _just_opened;
        out.just_closed = _just_closed;
        _just_opened    = false;
        _just_closed    = false;

        if (!player.player->progression.level_choice_pending()) {
            if (_screen_open) {
                _screen_open     = false;
                out.screen_open  = false;
                out.just_closed  = true;
            }
            return out;
        }

        if (!_screen_open) {
            _screen_open     = true;
            _selected_index  = 0;
            out.screen_open  = true;
            out.just_opened  = true;
        }

        out.world_paused = true;

        if (input.up)
            _selected_index = (_selected_index + 4) % 5;
        if (input.down)
            _selected_index = (_selected_index + 1) % 5;

        if (input.confirm && player.player->progression.level_choice_pending()) {
            switch (_selected_index) {
            case 0: player.player->attributes.vigor++;        break;
            case 1: player.player->attributes.forca++;        break;
            case 2: player.player->attributes.destreza++;     break;
            case 3: player.player->attributes.inteligencia++; break;
            case 4: player.player->attributes.endurance++;    break;
            default: break;
            }
            out.applied_index = _selected_index;
            player.player->progression.pending_level_ups--;

            recompute_player_derived_stats(
                player.derived,
                player.player->attributes,
                player.player->progression,
                player.player->talents,
                player.player->equipment,
                g_player_config.melee_damage,
                g_player_config.ranged_damage);

            const int base_hp = stress_mode ? 10000 : g_player_config.base_hp;
            player.health.max_hp = base_hp + player.derived.hp_max_bonus;
            if (player.health.current_hp > player.health.max_hp)
                player.health.current_hp = player.health.max_hp;
            player.mana.max    = g_player_config.base_mana_max    + player.derived.mana_max_bonus;
            player.stamina.max = g_player_config.base_stamina_max + player.derived.stamina_max_bonus;

            out.should_save = true;
        }

        if (!player.player->progression.level_choice_pending()) {
            _screen_open    = false;
            out.screen_open = false;
            out.just_closed = true;
        }

        if (input.back || input.pause) {
            _screen_open    = false;
            out.screen_open = false;
            out.just_closed = true;
        }

        return out;
    }

    void render(const Actor& player,
                SDL_Renderer* r,
                int viewport_w,
                int viewport_h) const
    {
        if (!_screen_open)
            return;
        render_attribute_screen(r,
                                viewport_w,
                                viewport_h,
                                player.player->attributes,
                                player.player->progression.pending_level_ups,
                                _selected_index,
                                _locale);
    }

    bool is_open() const { return _screen_open; }
    int selected_index() const { return _selected_index; }

private:
    bool _screen_open    = false;
    int  _selected_index = 0;
    bool _just_opened    = false;
    bool _just_closed    = false;
    LocaleSystem* _locale = nullptr;
};

} // namespace mion

