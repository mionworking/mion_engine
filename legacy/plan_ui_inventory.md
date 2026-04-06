# Plano: UI System e Inventário (MUIS)

Sistema de UI centralizado + inventário completo estilo Diablo (bag + slots de equip) com quickslot de poção automático.

---

## Visão Geral

- **11 slots de equipamento** fixos (armadura, armas, acessórios)
- **Bag** — grid de itens acumulados (Modelo B: bag separada dos slots)
- **Poção** — quickslot único com stack + cooldown + auto-fill (estilo Diablo 3, sem gerenciamento manual)
- **Tela de personagem** (`C`) — paperdoll no centro, slots ao redor, bag lateral
- **Input** — padrão existente (`OverlayInputEdges` + cascata na `WorldScene`) sem precisar de InputContext formal

---

## Slots de Equipamento (11)

| Categoria | Slot |
|---|---|
| Armadura | Cabeça, Tronco, Pernas, Pés, Mãos |
| Acessórios | Cinto, Amuleto, Anel Esquerdo, Anel Direito |
| Armas | Mão Principal, Mão Secundária |

---

## Fase 1 — ui::Theme (Centralização de Estilos)

### O que fazer
Adicionar `ui::Theme` em `src/core/ui.hpp` com todas as cores e métricas nomeadas.
Um único ponto de edição: mudar o tema muda o visual de todas as telas.

Estrutura mínima:
```cpp
namespace mion::ui {
struct Theme {
    // Painéis
    SDL_Color panel_bg        = {10, 8, 20, 220};
    SDL_Color panel_border    = {180, 160, 100, 255};
    int       panel_border_px = 2;

    // Texto
    SDL_Color text_normal     = {230, 230, 210, 255};
    SDL_Color text_selected   = {255, 220, 60, 255};
    SDL_Color text_disabled   = {100, 95, 85, 255};
    SDL_Color text_title      = {255, 220, 80, 255};
    SDL_Color text_hint       = {200, 200, 180, 255};

    // Highlight de seleção em listas
    SDL_Color list_highlight_bg     = {50, 45, 70, 200};
    SDL_Color list_highlight_border = {255, 210, 80, 255};

    // ProgressBar
    SDL_Color bar_bg     = {40, 30, 30, 200};
    SDL_Color bar_hp     = {200, 60, 60, 255};
    SDL_Color bar_mana   = {60, 100, 200, 255};
    SDL_Color bar_stamina= {60, 180, 80, 255};
    SDL_Color bar_border = {120, 100, 80, 180};
};

inline Theme g_theme; // instância global editável
} // namespace mion::ui
```

### Onde mexer
- `src/core/ui.hpp` — adicionar `Theme` e `g_theme`
- `src/systems/attribute_screen_ui.hpp` — substituir cores inline por `ui::g_theme.*`
- `src/systems/skill_tree_ui.hpp` — idem
- `src/core/pause_menu.hpp` (render) — idem
- Render do shop — idem

### Cuidados
- Não alterar a API de `Panel`, `Label`, `List`, `ProgressBar` — eles continuam funcionando; só seus defaults passam a vir do tema
- Defaults do `g_theme` devem ser as cores já existentes hoje — nenhuma tela muda de visual sem intenção
- Não adicionar animações nessa fase — foco só em centralizar

---

## Fase 2 — Expansão para 11 Slots de Equipamento

### O que fazer
Expandir o enum `EquipSlot` e `EquipmentState` de 3 para 11 slots.
Adicionar campo `icon_id` em `ItemDef` para UI (referência à textura do item).
Criar `data/equipment.ini` para definição data-driven dos itens (nome, slot, modificadores).
Bump do save para v7 com migração segura dos 3 slots antigos.

Novos slots em `src/components/equipment.hpp`:
```cpp
enum class EquipSlot {
    Head = 0, Chest, Legs, Feet, Hands,    // Armadura (5)
    Belt, Amulet, RingLeft, RingRight,      // Acessórios (4)
    MainHand, OffHand,                      // Armas (2)
    Count
};
inline constexpr int kEquipSlotCount = static_cast<int>(EquipSlot::Count); // 11
```

### Onde mexer
- `src/components/equipment.hpp` — enum + `kEquipSlotCount` + `EquipmentState`
- `src/core/save_data.hpp` — novo campo `std::string equipped_names[11]` + bump `kSaveFormatVersion = 7`
- `src/core/save_migration.hpp` — nova `migrate_v6_to_v7`: mapear slots antigos (Weapon→MainHand, Armor→Chest, Accessory→Amulet), demais slots começam vazios
- `src/core/save_system.hpp` — serializar/deserializar os 11 slots por nome
- `data/equipment.ini` — criar arquivo com definições de todos os itens

### Cuidados
- **Migração crítica**: slots antigos têm índices 0, 1, 2 — mapear explicitamente para os novos índices corretos. Não assumir que a ordem coincide
- Salvar apenas o **nome** do item no save, não toda a `ItemDef` — ao carregar, buscar a def no INI pelo nome
- Se um nome salvo não existir no INI, tratar como slot vazio (item deletado/renomeado entre versões)
- `ItemModifiers` não muda — compatível com os novos slots

---

## Fase 3 — Bag System (Inventário de Itens)

### O que fazer
Criar `ItemBag` — grid de N slots (ex: 4×6 = 24 slots) que acumula itens encontrados no mundo.
Pickup de item → vai para o primeiro slot vazio da bag.
Bag faz parte do `Actor` ou do estado do `WorldScene`.

Estrutura em `src/components/item_bag.hpp`:
```cpp
inline constexpr int kBagCols = 4;
inline constexpr int kBagRows = 6;
inline constexpr int kBagSize = kBagCols * kBagRows; // 24

struct ItemBag {
    std::string slots[kBagSize]; // nome do item ("" = vazio)

    int first_empty() const;          // -1 se cheia
    bool add(const std::string& name); // false se cheia
    void remove(int idx);
    bool is_empty(int idx) const { return slots[idx].empty(); }
};
```

### Onde mexer
- Novo `src/components/item_bag.hpp`
- `src/entities/actor.hpp` — adicionar `ItemBag bag`
- `src/core/save_data.hpp` — serializar bag (array de strings, v8 ou junto com v7)
- `src/core/save_migration.hpp` — bag vazia para saves sem o campo
- Sistema de loot/pickup (onde itens são dropados no mundo) — rota item→bag ao colidir com pickup

### Cuidados
- **Bag cheia**: definir comportamento agora — sugestão: item fica no chão, HUD mostra aviso "bag cheia"
- Itens do tipo poção **não vão para a bag** — são roteados direto para o `PotionQuickslot` (Fase 5)
- Salvar só o nome do item, não a `ItemDef` — a def vem do INI na carga
- Bag não tem stack para equipamentos — cada slot = um item único

---

## Fase 4 — Equipment Screen (Overlay 'C')

### O que fazer
`EquipmentScreenController` seguindo o padrão exato de `skill_tree_controller.hpp`:
- Flag `_screen_open`
- Estado interno com enum `State { BrowseSlots, BrowseBag, ActionMenu }`
- Retorna `EquipmentResult { bool world_paused, bool should_save }`
- Usa `OverlayInputEdges` — sem novo sistema de input

Layout da tela:
- Esquerda: bag grid (4×6)
- Centro: paperdoll do personagem com os 11 slots posicionados ao redor
- Direita: painel de stats (atual vs. item selecionado)

Sub-menu contextual (quando confirma num slot de equip ou item da bag):
- "Equipar" / "Desequipar" / "Inspecionar" — resolvido com estado interno, não com novo overlay

Game feel:
- Highlight animado ao navegar entre slots (pulse ou brilho)
- Fade-in do overlay ao abrir
- Som ao equipar/desequipar
- Painel de comparação de stats mostra delta (+3 DEF, -1 DMG) em tempo real ao navegar

Navegação:
- D-pad/setas navegam dentro da área focada (bag ou slots)
- Botão (ex: Tab ou L/R) troca o foco entre bag ↔ slots de equip

### Onde mexer
- Novo `src/systems/equipment_screen_controller.hpp`
- Novo `src/systems/equipment_screen_ui.hpp` (render separado)
- `src/scenes/world_scene.hpp` — adicionar na cascata do `fixed_update()` após o bloco de SkillTree, antes do Shop
- `src/core/input.hpp` — verificar/adicionar tecla 'C' mapeada para `inventory_pressed`
- `src/core/register_scenes.cpp` ou equivalente — instanciar o controller

### Cuidados
- **Input**: o sub-menu contextual é estado interno do controller — não cria nova entrada na cascata da WorldScene
- **Navegação entre áreas**: manter explícito qual área está focada (`_focus = BrowseBag | BrowseSlots`) para não misturar input
- **`should_save = true`** quando equipa ou desequipa — world_scene já tem padrão de trigger de save ao receber esse flag
- **Stat comparison**: calcular delta em tempo real (`hoveredItem.modifiers - equippedSlot.modifiers`) mas só exibir quando há item em ambos os lados da comparação
- **Fade-in**: implementar com um float `_open_alpha` que vai de 0→1 em alguns frames — não bloquear input durante o fade
- **Tamanho da bag no render**: cuidar para não sair da área de jogo (vw/vh) — calcular posições em função do viewport, não hardcoded

---

## Fase 5 — Potion Quickslot (Estilo Diablo 3)

### O que fazer
Um único slot de poção com stack count, qualidade e cooldown.
Lógica: pegar poção no mundo → se de qualidade superior, substitui a atual; senão, incrementa stack.
Pressionar tecla → usa poção → aplica heal → cooldown começa.
Render no HUD ao lado das barras de HP/Mana.

Estrutura em `src/components/potion_quickslot.hpp`:
```cpp
enum class PotionQuality { Minor = 0, Normal, Greater, Count };

struct PotionQuickslot {
    PotionQuality quality    = PotionQuality::Normal;
    int           stack      = 0;          // 0 = sem poção
    float         cooldown   = 0.f;        // segundos restantes
    float         max_cooldown = 30.f;     // ajustável por item/talento

    bool can_use() const { return stack > 0 && cooldown <= 0.f; }
    void use(Actor& player);               // aplica heal, decrementa stack, inicia cooldown
    void update(float dt);                 // decrementa cooldown
    void pickup(PotionQuality q, int amount); // auto-upgrade ou incrementa stack
};
```

### Onde mexer
- Novo `src/components/potion_quickslot.hpp`
- `src/entities/actor.hpp` — adicionar `PotionQuickslot potion`
- HUD render (onde HP/Stamina/Mana bars estão) — adicionar quickslot com ícone + stack count + cooldown indicator
- `src/core/save_data.hpp` — salvar `potion_quality` + `potion_stack`
- `src/core/save_migration.hpp` — stack = 0 para saves antigos
- Loot/pickup — detectar item do tipo poção → rota para `actor.potion.pickup()`, não para bag
- `src/scenes/world_scene.hpp` — chamar `player.potion.update(dt)` no fixed_update e processar input da tecla de poção

### Cuidados
- **Cooldown em tempo real**: usar `dt` acumulado em segundos, não ticks — consistente com o resto do engine
- **Stack máximo**: definir cap (99 é seguro) para não ter overflow visual no HUD
- **Tecla de poção**: verificar se já existe mapeamento em `src/core/input.hpp` — adicionar se não existir
- **Auto-upgrade**: só substitui se `q > quality` — sempre preservar a poção de maior qualidade disponível
- **Sem poção na bag**: o pickup detecta tipo poção antes de tentar inserir na bag

---

## Fase 6 — Consolidação dos Overlays Existentes

### O que fazer
Aplicar `ui::g_theme` nos overlays existentes que ainda usam cores inline:
- Pause menu
- Skill tree
- Attribute screen
- Shop

Verificar consistência visual: bordas, transparências, escala de fonte.

### Onde mexer
- `src/systems/skill_tree_ui.hpp`
- `src/systems/attribute_screen_ui.hpp`
- `src/core/pause_menu.hpp` (bloco de render)
- Render do shop (localizar arquivo exato)

### Cuidados
- Fase puramente visual — zero mudança de lógica
- Testar cada overlay individualmente após substituição
- Não quebrar padrões de input existentes — só o render é tocado

---

## Ordem de Implementação

```
Fase 1 (Theme)
  └─ Fase 2 (11 Slots + Save v7)
       └─ Fase 3 (Bag)
            └─ Fase 4 (Equipment Screen)
                 └─ Fase 5 (Potion Quickslot)
                      └─ Fase 6 (Consolidação)
```

Cada fase depende da anterior. Fase 6 pode ser feita em paralelo após a Fase 1.
