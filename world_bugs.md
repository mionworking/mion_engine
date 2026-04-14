# World Scene — Bugs corrigidos e pendentes

Data: 2026-04-13

---

## Bugs corrigidos

### Bug 1 — Personagens atravessam paredes entre town e dungeon

**Causa raiz (3 pontos):**

1. **Tilemap do corredor com paredes nos lados errados.**
   `corridor.tilemap.fill` preenchia as colunas 0 e 5 (esquerda/direita) como `Wall`. Para uma passagem horizontal (jogador entra pela esquerda, sai pela direita), as paredes visuais devem estar nas linhas 0 e N-1 (topo/fundo). Corrigido em `world_map_builder.hpp`.

2. **Corredor sem obstáculos físicos.**
   `corridor.room.obstacles.clear()` deixava a passagem sem colisão real. Qualquer ator podia sair acima ou abaixo do corredor (global y < 300 ou y > 1200) e aterrissar em "espaço vazio". Corrigido adicionando `corridor_top` e `corridor_bottom` em `world_map_builder.hpp`.

3. **Borda direita da town sem parede física.**
   A town não tinha obstáculos na borda x ≈ 2400. O jogador podia cruzar para fora da abertura do corredor (acima do y global 300 ou abaixo do 1200) sem nenhuma resistência. Corrigido adicionando `town_wall_upper` e `town_wall_lower` em `world_map_builder.hpp`.

4. **Quartos de dungeon sem paredes de borda no topo/fundo.**
   `RoomManager::build_room` preenche tiles `Wall` nas 4 bordas do tilemap, mas o sistema de colisão usa `room.obstacles`, não tiles. As bordas eram só visuais. Corrigido adicionando `border_top` e `border_bottom` a cada quarto em `world_map_builder.hpp`.

**Arquivos alterados:**
- `src/world/world_map_builder.hpp`

---

### Bug 2 — Diálogos da dungeon não disparam (prologue, room2, deeper, Grimjaw)

**Causa raiz:**
`_fresh_run()` resetava quest, ouro, progressão e posição — mas **não** resetava `_area_entry`. O `AreaEntrySystem` guarda um `std::set<ZoneId> visited_areas`. Se o jogador completou uma run anterior, todos os quartos já estão no set. Na nova run, o callback de `AreaEntrySystem::update` retorna cedo (`visited_areas.count(zone) != 0`) e não dispara nem diálogo nem spawn de inimigos.

O mesmo problema afeta o path de `Continue` somente se `visited_area_mask` no save já tem todos os bits setados (run completa). Nesse caso o comportamento é correto — o jogador já viu os diálogos. O bug era exclusivo do `New Game`.

**Fix:**
```cpp
// src/scenes/world_scene.hpp — _fresh_run()
_area_entry = {};   // reset visited areas so dialogues/spawns fire on new run
```

**Arquivo alterado:**
- `src/scenes/world_scene.hpp`

---

## O que ainda falta (paredes entre quartos de dungeon)

As paredes **entre quartos de dungeon adjacentes** (direita do Room 0 / esquerda do Room 1, etc.) ainda não têm colisão. O jogador pode atravessar livremente entre quartos pela borda x — a abertura visual da "porta" (definida pelos tiles na borda) não bloqueia fisicamente.

### Por que não foi corrigido agora

É uma decisão de design antes de ser um bug de implementação:

- No open-world layout atual, os quartos são diretamente adjacentes sem área de transição. Adicionar paredes de borda esquerda/direita em cada quarto criaria uma parede sólida — o jogador precisaria de um "gap" exato alinhado com a abertura da porta.
- `RoomManager::build_room` define o gap das portas laterais em `midy ± 56px` (`midy = (bounds.min_y + bounds.max_y) / 2`). Para todos os quartos 0–2 (900px de altura): `midy = 450`, gap = y 394–506.
- O boss room (1050px): `midy = 525`, gap = y 469–581.

### Plano de implementação quando decidir fechar

Para cada quarto de dungeon (ri = 0–3), em `WorldMapBuilder::build`, adicionar após `border_top/bottom`:

```cpp
// Parede esquerda com gap para a porta (todos os quartos)
const float midy = (rb.min_y + rb.max_y) * 0.5f;
constexpr float kDoor = 56.f;   // metade da abertura da porta
constexpr float kLW   = 32.f;   // espessura da parede (1 tile)

// Quarto 0: esquerda aberta (corredor ocupa 100% da altura) — sem parede esquerda
// Quartos 1–3: parede esquerda com gap no midy
if (ri > 0) {
    area.room.add_obstacle("left_wall_upper", rb.min_x, rb.min_y,       rb.min_x + kLW, midy - kDoor);
    area.room.add_obstacle("left_wall_lower", rb.min_x, midy + kDoor,   rb.min_x + kLW, rb.max_y);
}

// Parede direita com gap no midy (quartos 0–2 têm porta p/ próximo quarto)
// Quarto 3 (boss): pode ter ou não, dependendo do design da saída
if (ri < 3) {
    area.room.add_obstacle("right_wall_upper", rb.max_x - kLW, rb.min_y,      rb.max_x, midy - kDoor);
    area.room.add_obstacle("right_wall_lower", rb.max_x - kLW, midy + kDoor,  rb.max_x, rb.max_y);
}
```

**Nota:** O corredor (192px) é mais estreito que a abertura da porta (112px). Se quiser que o jogador entre no corredor centralizado, pode reduzir a abertura do Room 0 para `midy ± 56` também — mas isso exige testar a sensação de jogo primeiro.

---

## Arquivos alterados nesta sessão

| Arquivo | Mudança |
|---|---|
| `src/scenes/world_scene.hpp` | `_fresh_run()` reseta `_area_entry` |
| `src/world/world_map_builder.hpp` | Tilemap do corredor corrigido; obstáculos físicos em town, corredor e quartos de dungeon |
