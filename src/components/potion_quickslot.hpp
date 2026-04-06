#pragma once
#include "../components/health.hpp"

namespace mion {

// ---------------------------------------------------------------------------
// PotionQuickslot — sistema estilo Diablo 3.
// Um único slot com stack + cooldown + auto-upgrade de qualidade.
// Pickup de poção no mundo alimenta este slot automaticamente.
// Não requer gerenciamento manual do jogador.
// ---------------------------------------------------------------------------

enum class PotionQuality { Minor = 0, Normal, Greater };

struct PotionQuickslot {
    PotionQuality quality      = PotionQuality::Normal;
    int           stack        = 0;       // quantidade disponível (0 = sem poção)
    float         cooldown     = 0.0f;   // segundos restantes de cooldown
    float         max_cooldown = 30.0f;  // cooldown total após uso

    static constexpr int kMaxStack = 99;

    bool can_use() const { return stack > 0 && cooldown <= 0.0f; }

    // Quantidade de HP restaurada por qualidade.
    int heal_amount() const {
        switch (quality) {
            case PotionQuality::Minor:   return 30;
            case PotionQuality::Normal:  return 60;
            case PotionQuality::Greater: return 120;
        }
        return 60;
    }

    // Usa a poção: aplica heal, decrementa stack, inicia cooldown.
    // Retorna true se usou com sucesso.
    bool use(HealthState& health) {
        if (!can_use()) return false;
        health.current_hp = health.current_hp + heal_amount();
        if (health.current_hp > health.max_hp)
            health.current_hp = health.max_hp;
        --stack;
        cooldown = max_cooldown;
        return true;
    }

    // Atualiza cooldown. Chamar a cada fixed_update com dt em segundos.
    void update(float dt) {
        if (cooldown > 0.0f) {
            cooldown -= dt;
            if (cooldown < 0.0f) cooldown = 0.0f;
        }
    }

    // Recebe poção do mundo. Se melhor, substitui e adiciona ao stack.
    // Se igual ou menor, apenas incrementa stack.
    void pickup(PotionQuality q, int amount = 1) {
        if (q > quality) {
            quality = q;
        }
        stack += amount;
        if (stack > kMaxStack) stack = kMaxStack;
    }

    // Fração do cooldown restante (0.0 = pronto, 1.0 = recém-usado).
    float cooldown_ratio() const {
        if (max_cooldown <= 0.0f) return 0.0f;
        return cooldown / max_cooldown;
    }
};

} // namespace mion
