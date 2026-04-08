#pragma once

#include "../core/audio.hpp"
#include "../entities/actor.hpp"
#include "../entities/shop.hpp"
#include "overlay_input.hpp"
#include "shop_system.hpp"
#include "ui_controller_contract.hpp"

namespace mion {

using ShopInputResult = UiControllerResult;

// Stateful edge tracker for shop UI input.
// Owns prev-frame state for move_y, confirm and cancel so callers
// don't need to keep those variables externally.
class ShopInputController {
public:
    // Process one tick of shop input. Mutates inventory selection, player gold
    // and shop_open. Audio is optional.
    ShopInputResult update(ShopInventory& inventory,
                           Actor& player,
                           const OverlayInputEdges& input,
                           AudioSystem* audio,
                           bool& shop_open)
    {
        ShopInputResult out{};
        out.world_paused = true;

        if (!inventory.items.empty()) {
            if (input.up && inventory.selected_index > 0)
                --inventory.selected_index;
            if (input.down && inventory.selected_index < static_cast<int>(inventory.items.size()) - 1)
                ++inventory.selected_index;
        }

        if (input.confirm && ShopSystem::try_buy(player, inventory, inventory.selected_index)) {
            out.should_save = true;
            if (audio)
                audio->play_sfx(SoundId::UiConfirm, 0.5f);
        }

        if (input.back)
            shop_open = false;

        return out;
    }

private:
    // Stateless by design: edge semantics are provided by OverlayInputEdges.
};

} // namespace mion
