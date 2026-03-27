#pragma once

namespace mion {

enum class AttackPhase {
    Idle,
    Startup,   // animação de antecipação — sem hitbox
    Active,    // janela de dano — hitbox ativa
    Recovery,  // vulnerável, sem controle total
};

// Espelho direto do CombatState Python — mesma lógica, mesma arquitetura
struct CombatState {
    // Hurt / invulnerabilidade
    float hurt_stun_duration_seconds         = 0.20f;
    float hurt_stun_remaining_seconds        = 0.0f;
    float impact_invulnerability_seconds     = 0.50f;
    float impact_invulnerability_remaining   = 0.0f;

    // Fases do ataque
    float attack_startup_duration_seconds    = 0.10f;
    float attack_active_duration_seconds     = 0.15f;
    float attack_recovery_duration_seconds   = 0.25f;

    AttackPhase attack_phase                 = AttackPhase::Idle;
    float       attack_phase_remaining       = 0.0f;
    bool        attack_hit_landed            = false;

    // --- Queries ---
    bool is_hurt_stunned()      const { return hurt_stun_remaining_seconds > 0.0f; }
    bool can_receive_hit()      const { return impact_invulnerability_remaining <= 0.0f; }
    bool is_attack_idle()       const { return attack_phase == AttackPhase::Idle; }
    bool is_in_active_phase()   const { return attack_phase == AttackPhase::Active; }

    // --- Commands ---
    void begin_attack() {
        if (!is_attack_idle()) return;
        attack_phase           = AttackPhase::Startup;
        attack_phase_remaining = attack_startup_duration_seconds;
        attack_hit_landed      = false;
    }

    void advance_time(float dt) {
        if (hurt_stun_remaining_seconds > 0.0f)
            hurt_stun_remaining_seconds -= dt;
        if (impact_invulnerability_remaining > 0.0f)
            impact_invulnerability_remaining -= dt;

        if (attack_phase == AttackPhase::Idle) return;

        attack_phase_remaining -= dt;
        if (attack_phase_remaining <= 0.0f) {
            switch (attack_phase) {
                case AttackPhase::Startup:
                    attack_phase           = AttackPhase::Active;
                    attack_phase_remaining = attack_active_duration_seconds;
                    break;
                case AttackPhase::Active:
                    attack_phase           = AttackPhase::Recovery;
                    attack_phase_remaining = attack_recovery_duration_seconds;
                    break;
                case AttackPhase::Recovery:
                    attack_phase           = AttackPhase::Idle;
                    attack_phase_remaining = 0.0f;
                    break;
                default: break;
            }
        }
    }

    void apply_hit_reaction() {
        hurt_stun_remaining_seconds      = hurt_stun_duration_seconds;
        impact_invulnerability_remaining = impact_invulnerability_seconds;
        // interrompe ataque em curso
        attack_phase           = AttackPhase::Idle;
        attack_phase_remaining = 0.0f;
    }

    void reset_for_spawn() {
        hurt_stun_remaining_seconds      = 0.0f;
        impact_invulnerability_remaining = 0.0f;
        attack_phase                     = AttackPhase::Idle;
        attack_phase_remaining           = 0.0f;
        attack_hit_landed                = false;
    }
};

} // namespace mion
