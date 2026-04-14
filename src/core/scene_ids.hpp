#pragma once

namespace mion {

// IDs de cena tipados. Compilador detecta typo em vez de falha silenciosa em runtime.
// Strings só na borda: internals do registry e serialização de save.
enum class SceneId {
    kNone,      // sentinela: nenhuma transição pendente
    kTitle,
    kWorld,
    kGameOver,
    kVictory,
    kCredits,
    kQuit,      // sentinela: encerrar a aplicação
};

// Conversão para string — uso exclusivo no registry e em mensagens de log.
inline const char* to_string(SceneId id) {
    switch (id) {
        case SceneId::kTitle:    return "title";
        case SceneId::kWorld:    return "world";
        case SceneId::kGameOver: return "game_over";
        case SceneId::kVictory:  return "victory";
        case SceneId::kCredits:  return "credits";
        case SceneId::kQuit:     return "__quit__";
        default:                 return "";
    }
}

} // namespace mion
