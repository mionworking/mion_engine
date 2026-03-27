#pragma once
#include <cmath>

namespace mion {

struct InputState {
    float move_x        = 0.0f;  // -1.0 a +1.0
    float move_y        = 0.0f;  // -1.0 a +1.0
    bool  attack_pressed = false;

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
    InputState read_state() const override;  // implementado em input.cpp
};

} // namespace mion
