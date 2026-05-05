#pragma once
#include <cstdint>

namespace mion {

// Karma: recurso narrativo que substitui XP como fonte de level (Sprint K1.3).
// total:     acumulado na vida (nunca decresce) — drives level.
// available: saldo gastável em focus mode (Sprint K1.4) e futuras mecânicas.
struct KarmaData {
    int64_t total     = 0;
    int64_t available = 0;
};

inline void karma_add(KarmaData& k, int64_t amount) {
    if (amount <= 0) return;
    k.total     += amount;
    k.available += amount;
}

inline bool karma_spend(KarmaData& k, int64_t amount) {
    if (amount <= 0 || k.available < amount) return false;
    k.available -= amount;
    return true;
}

inline void karma_refund(KarmaData& k, int64_t amount) {
    if (amount <= 0) return;
    k.available += amount;
}

} // namespace mion
