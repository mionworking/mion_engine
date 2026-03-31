#pragma once
#include <cmath>

#include "keybind_config.hpp"

namespace mion {

struct InputState {
    float move_x         = 0.0f;  // -1.0 a +1.0
    float move_y         = 0.0f;  // -1.0 a +1.0
    bool  attack_pressed  = false;
    bool  confirm_pressed = false; // Enter — diálogo / UI (não ataque)
    bool  ui_cancel_pressed = false; // Backspace — fechar loja / UI
    bool  pause_pressed       = false; // Escape — pausa (cena interpreta edge)
    bool  cancel_pressed      = false; // mesmo teclado que pause; overlay fecha
    bool  ui_up_pressed       = false; // setas ↑ (menu)
    bool  ui_down_pressed     = false; // setas ↓
    bool  ui_left_pressed     = false; // setas ←
    bool  ui_right_pressed    = false; // setas →
    bool  skill_tree_pressed  = false; // Tab — árvore de talentos
    bool  erase_save_pressed = false; // N — apagar save no title
    bool  dash_pressed   = false;  // LShift — Fase 1.1
    bool  ranged_pressed = false;  // X      — Fase 1.3
    bool  parry_pressed  = false;  // C
    bool  spell_1_pressed = false; // Q
    bool  spell_2_pressed = false; // E
    bool  spell_3_pressed = false; // R
    bool  spell_4_pressed = false; // F
    bool  upgrade_1      = false;  // teclas de level-up (1/2/3)
    bool  upgrade_2      = false;
    bool  upgrade_3      = false;
    bool  talent_1_pressed = false; // 4
    bool  talent_2_pressed = false; // 5
    bool  talent_3_pressed = false; // 6

    bool is_moving() const {
        return move_x != 0.0f || move_y != 0.0f;
    }

    // Vetor normalizado — garante velocidade igual em diagonais
    void normalized_movement(float& out_x, float& out_y) const {
        float len = std::sqrt(move_x * move_x + move_y * move_y);
        if (len > 0.0f) {
            out_x = move_x / len;
            out_y = move_y / len;
        } else {
            out_x = 0.0f;
            out_y = 0.0f;
        }
    }
};

// Interface — PlayerController lê daqui, não do SDL diretamente
// Permite trocar por ScriptedInputSource nos testes sem mudar nada
class IInputSource {
public:
    virtual ~IInputSource() = default;
    virtual InputState read_state() const = 0;
};

// Leitura real do teclado via SDL3
class KeyboardInputSource : public IInputSource {
public:
    explicit KeyboardInputSource(const KeybindConfig& kb = {}) : _kb(kb) {}
    InputState read_state() const override;

private:
    KeybindConfig _kb;
};

// Leitura de gamepad via SDL3 (API SDL_Gamepad)
class GamepadInputSource : public IInputSource {
public:
    GamepadInputSource() = default;
    ~GamepadInputSource();

    bool is_connected() const;
    void try_connect();
    InputState read_state() const override;

private:
    SDL_Gamepad* _gamepad = nullptr;
};

} // namespace mion
