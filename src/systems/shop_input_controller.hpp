#pragma once

#include "../core/audio.hpp"
#include "../core/input.hpp"
#include "../entities/actor.hpp"
#include "../entities/shop.hpp"
#include "shop_system.hpp"

namespace mion {

namespace ShopInputController {

inline void update_shop_input(ShopInventory& inventory,
                              Actor& player,
                              const InputState& input,
                              AudioSystem* audio,
                              bool& shop_open,
                              float& prev_move_y,
                              bool& prev_confirm,
                              bool& prev_cancel) {
    constexpr float kThreshold = 0.35f;

    if (!inventory.items.empty()) {
        if (input.move_y < -kThreshold && prev_move_y >= -kThreshold && inventory.selected_index > 0)
            --inventory.selected_index;
        if (input.move_y > kThreshold && prev_move_y <= kThreshold
            && inventory.selected_index < static_cast<int>(inventory.items.size()) - 1)
            ++inventory.selected_index;
    }
    prev_move_y = input.move_y;

    const bool buy_edge = input.confirm_pressed && !prev_confirm;
    if (buy_edge && ShopSystem::try_buy(player, inventory, inventory.selected_index) && audio)
        audio->play_sfx(SoundId::UiConfirm, 0.5f);

    if (input.ui_cancel_pressed && !prev_cancel)
        shop_open = false;

    prev_confirm = input.confirm_pressed;
    prev_cancel  = input.ui_cancel_pressed;
}

} // namespace ShopInputController

} // namespace mion
