#pragma once

#include <SDL3/SDL.h>
#include <vector>

#include "../core/locale.hpp"
#include "../entities/actor.hpp"
#include "../components/talent_tree.hpp"
#include "overlay_input.hpp"
#include "skill_tree_ui.hpp"
#include "ui_controller_contract.hpp"

namespace mion {

struct SkillTreeResult : UiControllerResult {
    bool screen_open = false;
};

inline bool skill_tree_selection_pending(const Actor& player) {
    return player.player->talents.pending_points > 0
        && player.player->talents.has_unlockable_options();
}

inline void skill_tree_build_columns(std::vector<int> out[3]) {
    for (int c = 0; c < 3; ++c)
        out[c].clear();
    for (int i = 0; i < kTalentCount; ++i) {
        const int disc = static_cast<int>(g_talent_nodes[static_cast<size_t>(i)].discipline);
        if (disc >= 0 && disc < 3)
            out[disc].push_back(i);
    }
}

inline void skill_tree_clamp_cursor(int& selected_col,
                                    int& selected_row,
                                    const std::vector<int> col_indices[3]) {
    if (selected_col < 0)
        selected_col = 0;
    if (selected_col > 2)
        selected_col = 2;
    const int nrows = static_cast<int>(col_indices[selected_col].size());
    if (nrows <= 0) {
        selected_row = 0;
        return;
    }
    if (selected_row >= nrows)
        selected_row = nrows - 1;
    if (selected_row < 0)
        selected_row = 0;
}

inline bool skill_tree_try_spend_talent(Actor& player, TalentId id) {
    if (!player.player->talents.try_unlock(id))
        return false;
    if (id == TalentId::ArcaneReservoir) {
        player.mana.max += 30.0f;
        player.mana.current += 30.0f;
        if (player.mana.current > player.mana.max)
            player.mana.current = player.mana.max;
    } else if (id == TalentId::ManaFlow) {
        player.mana.regen_rate += 8.0f;
    }
    player.player->spell_book.sync_from_talents(player.player->talents);
    return true;
}

class SkillTreeController {
public:
    void set_locale(LocaleSystem* locale) {
        _locale = locale;
    }

    void rebuild_columns() {
        skill_tree_build_columns(_col_indices);
        skill_tree_clamp_cursor(_selected_col, _selected_row, _col_indices);
    }

    void open() {
        _ensure_columns();
        _screen_open   = true;
        _selected_col  = 0;
        _selected_row  = 0;
        skill_tree_clamp_cursor(_selected_col, _selected_row, _col_indices);
    }

    SkillTreeResult update(Actor& player,
                           const OverlayInputEdges& input,
                           bool stress_mode)
    {
        _ensure_columns();

        SkillTreeResult out{};
        out.screen_open = _screen_open;
        bool just_opened = false;

        const bool pending = skill_tree_selection_pending(player);
        if (pending && !_screen_open) {
            open();
            just_opened = true;
        } else if (!pending && !_screen_open && input.tab) {
            open();
            just_opened = true;
        }

        out.screen_open = _screen_open;
        if (!_screen_open)
            return out;

        out.world_paused = true;

        if (!just_opened && (input.pause || input.tab || input.back)) {
            _screen_open    = false;
            out.screen_open = false;
            return out;
        }

        if (input.left) {
            _selected_col = (_selected_col + 2) % 3;
            skill_tree_clamp_cursor(_selected_col, _selected_row, _col_indices);
        }
        if (input.right) {
            _selected_col = (_selected_col + 1) % 3;
            skill_tree_clamp_cursor(_selected_col, _selected_row, _col_indices);
        }

        const int nrows = static_cast<int>(_col_indices[_selected_col].size());
        if (input.up && nrows > 0)
            _selected_row = (_selected_row + nrows - 1) % nrows;
        if (input.down && nrows > 0)
            _selected_row = (_selected_row + 1) % nrows;

        if (input.confirm && !stress_mode && nrows > 0) {
            const auto& col = _col_indices[_selected_col];
            const int ti = col[static_cast<size_t>(_selected_row)];
            if (skill_tree_try_spend_talent(player, static_cast<TalentId>(ti)))
                out.should_save = true;
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
        render_skill_tree_overlay(r,
                                  viewport_w,
                                  viewport_h,
                                  player.player->talents,
                                  _selected_col,
                                  _selected_row,
                                  _col_indices,
                                  _locale);
    }

    bool is_open() const { return _screen_open; }
    int selected_col() const { return _selected_col; }
    int selected_row() const { return _selected_row; }

private:
    void _ensure_columns() {
        if (_col_indices[0].empty() && _col_indices[1].empty() && _col_indices[2].empty())
            rebuild_columns();
    }

    bool             _screen_open  = false;
    int              _selected_col = 0;
    int              _selected_row = 0;
    std::vector<int> _col_indices[3];
    LocaleSystem*    _locale = nullptr;
};

} // namespace mion
