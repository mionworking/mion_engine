#pragma once
#include <SDL3/SDL.h>

#include "../core/locale.hpp"
#include "../entities/actor.hpp"
#include "../systems/overlay_input.hpp"
#include "../systems/equipment_screen_ui.hpp"
#include "ui_controller_contract.hpp"

namespace mion {

using EquipmentResult = UiControllerResult;

class EquipmentScreenController {
public:
    void set_locale(LocaleSystem* locale) { _locale = locale; }

    void open() {
        _screen_open  = true;
        _open_alpha   = 0.0f;
        _focus_bag    = false;
        _slot_cursor  = 0;
        _bag_cursor   = 0;
        _ctx_open     = false;
        _ctx_selected = 0;
    }

    void close() { _screen_open = false; }
    bool is_open() const { return _screen_open; }

    EquipmentResult update(Actor& player,
                           const OverlayInputEdges& input,
                           float dt)
    {
        EquipmentResult out{};
        if (!_screen_open) {
            if (input.inventory) open();
            return out;
        }

        out.world_paused = true;

        // Fade-in
        if (_open_alpha < 1.0f) {
            _open_alpha += dt * 8.0f; // ~0.125s to fully open
            if (_open_alpha > 1.0f) _open_alpha = 1.0f;
        }

        // Context menu takes priority
        if (_ctx_open) {
            _update_context(player, input, out);
            return out;
        }

        // Close
        if (input.pause || input.back) {
            _screen_open = false;
            return out;
        }

        // Switch focus between slots and bag
        if (input.tab) {
            _focus_bag = !_focus_bag;
            return out;
        }

        if (_focus_bag) {
            _update_bag(player, input, out);
        } else {
            _update_slots(player, input, out);
        }

        return out;
    }

    void render(const Actor& player, SDL_Renderer* r, int vw, int vh) const {
        if (!_screen_open) return;
        EquipScreenRenderState state;
        state.focus_bag    = _focus_bag;
        state.slot_cursor  = _slot_cursor;
        state.bag_cursor   = _bag_cursor;
        state.ctx_open     = _ctx_open;
        state.ctx_selected = _ctx_selected;
        state.open_alpha   = _open_alpha;
        render_equipment_screen(r, vw, vh, player, state, _locale);
    }

private:
    bool  _screen_open  = false;
    float _open_alpha   = 0.0f;
    bool  _focus_bag    = false;
    int   _slot_cursor  = 0; // 0..kEquipSlotCount-1
    int   _bag_cursor   = 0; // 0..kBagSize-1
    bool  _ctx_open     = false;
    int   _ctx_selected = 0; // 0=action, 1=cancel
    LocaleSystem* _locale = nullptr;

    void _update_slots(Actor& player, const OverlayInputEdges& input, EquipmentResult& out) {
        if (input.up)
            _slot_cursor = (_slot_cursor + kEquipSlotCount - 1) % kEquipSlotCount;
        if (input.down)
            _slot_cursor = (_slot_cursor + 1) % kEquipSlotCount;

        if (input.confirm) {
            const EquipSlot s = static_cast<EquipSlot>(_slot_cursor);
            if (player.player->equipment.is_equipped(s)) {
                _ctx_open     = true;
                _ctx_selected = 0;
            }
            // Empty slot: nothing to do (equipping is done from bag side)
        }
    }

    void _update_bag(Actor& player, const OverlayInputEdges& input, EquipmentResult& out) {
        if (input.up)
            _bag_cursor = (_bag_cursor - kBagCols + kBagSize) % kBagSize;
        if (input.down)
            _bag_cursor = (_bag_cursor + kBagCols) % kBagSize;
        if (input.left)
            _bag_cursor = (_bag_cursor + kBagSize - 1) % kBagSize;
        if (input.right)
            _bag_cursor = (_bag_cursor + 1) % kBagSize;

        if (input.confirm && !player.player->bag.is_empty(_bag_cursor)) {
            _ctx_open     = true;
            _ctx_selected = 0;
        }
    }

    void _update_context(Actor& player, const OverlayInputEdges& input, EquipmentResult& out) {
        if (input.up || input.down)
            _ctx_selected = 1 - _ctx_selected; // toggle between action and cancel

        if (input.back || input.pause) {
            _ctx_open = false;
            return;
        }

        if (!input.confirm) return;

        if (_ctx_selected == 1) { // Cancel
            _ctx_open = false;
            return;
        }

        // Action selected
        if (_focus_bag) {
            // Equip from bag
            const BagSlot& bs = player.player->bag.slots[_bag_cursor];
            if (!bs.is_empty()) {
                const EquipSlot target = bs.slot;
                // If slot already occupied, move old item back to bag
                if (player.player->equipment.is_equipped(target)) {
                    const ItemDef& old = player.player->equipment.slots[static_cast<int>(target)];
                    player.player->bag.add(old.name, target);
                }
                // Equip new item
                ItemDef def;
                def.name  = bs.name;
                def.slot  = target;
                def.valid = true;
                player.player->equipment.equip(target, def);
                player.player->bag.remove(_bag_cursor);
                out.should_save = true;
            }
        } else {
            // Unequip to bag
            const EquipSlot s = static_cast<EquipSlot>(_slot_cursor);
            if (player.player->equipment.is_equipped(s)) {
                const ItemDef& item = player.player->equipment.slots[static_cast<int>(s)];
                if (!player.player->bag.is_full()) {
                    player.player->bag.add(item.name, s);
                    player.player->equipment.unequip(s);
                    out.should_save = true;
                }
                // If bag full: não faz nada (item permanece equipado)
            }
        }

        _ctx_open = false;
    }
};

} // namespace mion
