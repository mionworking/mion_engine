#pragma once

namespace mion {

// Layout lógico padrão para sprites de personagens estilo "Puny".
// Mantendo estes valores centralizados, fica fácil migrar de 32×32
// para 48×48 ou 64×64 no futuro sem caçar números mágicos no código.
struct SpriteLayout {
    static constexpr int PunyFrameW  = 32;
    static constexpr int PunyFrameH  = 32;
    static constexpr int PunyCols    = 24;
    static constexpr int PunyRows    = 8;
};

} // namespace mion

