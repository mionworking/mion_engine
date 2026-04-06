# Plano de Implementação: O Peso do Carma (Mion Engine)

Este documento serve como o guia definitivo para a inserção da narrativa e mecânicas de história na Mion Engine. O objetivo é criar um sistema coeso onde o "Estado do Mundo" dita o gameplay, evitando adições isoladas ("puxadinhos").

---

## 1. Mapeamento de Mudanças (Onde e o Quê)

### A. Core & State (`src/world/world_context.hpp`)
**O que muda:** Adição das variáveis de controle global da narrativa.
```cpp
// Esboço (Stub)
struct WorldContext {
    // ... campos existentes ...
    int   current_act  = 1;     // Controla comportamento de IA e Visuals
    float world_karma  = 0.0f;  // Nível de "Heat" para Arautos
    bool  is_mindscape = false; // Flag para efeitos de Render
};
```

### B. Entidades (`src/entities/actor.hpp`)
**O que muda:** Adição de atributos de Carma e Traumas.
```cpp
struct Actor {
    // ...
    float trauma_karma      = 0.0f;  // Carma acumulado/gerado por este ator
    bool  cc_resistance_act1 = false; // Recompensa do trauma da Infância
    // ...
};
```

### C. Sistemas de Status (`src/systems/status_effect_system.hpp`)
**O que muda:** Implementação de `Fear` e `Bleed`.
```cpp
// Fear: Interrompe movimento/ataque por X segundos
// Bleed: Dano por tick, escala se o player se mover
if (e.type == StatusEffectType::Fear) {
    a->move_speed *= 0.1f; // Paralisia quase total
}
```

### D. Inteligência Artificial (`src/systems/enemy_ai.hpp`)
**O que muda:** Comportamento variante por Ato.
```cpp
// Exemplo de lógica condicional
if (context.current_act == 1) {
    // Skirmisher: Ataca e foge (Hit-and-run)
} else if (context.current_act == 3) {
    // Swarm: Aumenta dano baseado em inimigos próximos
}
```

---

## 2. Controllers: A Espinha Dorsal

### [NEW] `NarrativeController`
Responsável por:
*   Monitorar `QuestState` para disparar transições de Ato.
*   Controlar o `NarrativeSequencer` para momentos cinematográficos.

### [NEW] `WorldStateManager`
Responsável por:
*   Mudar a paleta de cores do `TilemapRenderer`.
*   Alterar a música ambiente via `AudioSystem`.

---

## 3. Plano de Ação (Roadmap)

### Fase 1: Infraestrutura (Semana 1)
*   [ ] Modificar `WorldContext` e `Actor`.
*   [ ] Criar o `WorldStateManager` básico.
*   [ ] Implementar o recurso de Carma no `ResourceSystem`.

### Fase 2: Atos e Combate (Semana 2)
*   [ ] Implementar `AIBehavior::Skirmisher` (Ato I).
*   [ ] Adicionar efeitos de `Fear` e `Bleed` ao `StatusEffectSystem`.
*   [ ] Criar os spawners de Arautos baseados no nível de Carma.

### Fase 3: Mindscape e Narrativa (Semana 3)
*   [ ] Desenvolver o `MindscapeFX` (Shaders/Filtros).
*   [ ] Integrar diálogos com gatilhos de eventos mundiais.
*   [ ] Implementar as recompensas de "Resolução de Trauma".

---

## 4. Estereótipos de Inimigos (Arautos)

| Nome | Ato | Mecânica Principal | Trauma Associado |
| :--- | :--- | :--- | :--- |
| **Arauto do Medo** | I | Invisibilidade e Paralisia | Abandono (Infância) |
| **Arauto da Dor** | II | Tanque com Retaliação (AoE) | Perda Física (Adolescência) |
| **Arauto do Ódio** | III | Swarm e Berserk (Dano p/ vida baixa) | Ressentimento (Adulto) |

---

## 5. Perguntas Estratégicas para o Level Design
*   **Flashbacks**: Devem ser áreas instanciadas (salas pequenas) para máximo controle dramático.
*   **Progressão**: A morte de um Arauto desbloqueia permanentemente uma nova habilidade passiva no `TalentState`.

---
*Este plano deve ser seguido rigorosamente para garantir a integridade técnica da Mion Engine durante a expansão narrativa.*
