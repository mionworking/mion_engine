# mion_engine — Roadmap

## Feito

- [x] Loop de física fixo (accumulator pattern, 60fps desacoplado do render)
- [x] EngineConfig, Camera2D (follow + clamp + screen shake com decay linear)
- [x] IInputSource + KeyboardInputSource (WASD/arrows/Space/Z)
- [x] Transform2D, HealthState, AABB, CollisionBox, HurtBox, MeleeHitBox
- [x] CombatState (Idle → Startup → Active → Recovery, um hit por swing)
- [x] Actor struct unificado (Player/Enemy via Team enum)
- [x] RoomDefinition (WorldBounds + obstáculos estáticos)
- [x] RoomCollisionSystem (actor vs obstáculos + actor vs actor, push-out 3 iterações)
- [x] MeleeCombatSystem (hitbox ativa só na fase Active, friendly fire off)
- [x] EnemyAISystem (aggro → separação entre inimigos → perseguir → atacar)
- [x] HP bars, flash de dano, debug de hitbox e facing
- [x] Knockback — impulso + decaimento por fricção, corrigido pela colisão
- [x] Hitstop — 4 frames congelados no impacto (game feel)
- [x] Screen shake — câmera treme 8 frames com decay linear, só em hits do player
- [x] **Morte do player** — tela de game over, restart com Space após 1s
- [x] **Bitmap font** — texto 8×8 embedded (sem dependências), draw_text + text_width
- [x] **SceneSystem** — IScene (enter/exit/fixed_update/render/next_scene) + SceneManager
- [x] **Sprites** — TextureCache + draw_sprite via stb_image (PNG, sem submodulos)
- [x] **Tilemap** — chão e paredes com tiles 32px, culling por câmera, fill + borda de paredes
- [x] **Iluminação fake** — máscara escura + gradiente radial via SDL render target (estilo Diablo 1)
- [x] **Testes automatizados** — 70 asserts / 19 casos cobrindo todos os sistemas (sem framework externo)
- [x] Clangd configurado (.vscode/c_cpp_properties.json → compile_commands.json)

---

## Falta — Engine Core

- [ ] **SceneRegistry** — carrega cenas por nome (dungeons, menus)
- [ ] **SceneTransitionZone** — zona que troca de cena ao player entrar
- [ ] **ScriptedInputSource** — replay de inputs para testes de integração

- [x] **Spritesheet + animação** — `AnimPlayer` com clips por estado (Idle/Walk/Attack/Hurt/Death), fallback para retângulos, layout padrão por spritesheet
- [x] **Múltiplos tipos de inimigo** — `EnemyType` (Skeleton/Orc/Ghost), `EnemyDef` com todos os stats, spawn tipado via `_spawn_enemy()`
- [x] **Pathfinding A*** — `NavGrid` baked por sala, A* Manhattan 4-dir, replan por-actor escalonado (350ms), fallback linha reta
- [x] **SDL3 Audio** — `AudioSystem` com SFX fire-and-forget e música com looping; SFX: hit, attack, enemy_death, player_death; música: dungeon_ambient

---

## Falta — Gameplay

- [ ] **Itens no chão** — drop de inimigos, coleta por proximidade
- [ ] **Stats do player** — dano, velocidade, defesa configuráveis

---

## Falta — Render

- [ ] **Partículas simples** — sangue/faíscas no impacto

---

## Falta — Infraestrutura

- [ ] **Asset pipeline** — carregar texturas/sons de pasta `assets/`, sem hardcode
- [ ] **Save/load** — serialização simples de estado do player
- [ ] **Build release** — `make release`, binário otimizado
- [ ] **Windows cross-compile** — testar build MinGW ou MSVC
- [ ] **Config externa** — `config.ini` para resolução, volume, teclas

---

## Próximas sessões (sugestão)

1. **Assets reais** — adicionar sprites PNG e WAVs em `assets/sprites/` e `assets/audio/`
2. **Itens no chão** — drops de inimigos
3. **Asset pipeline** — carregamento externo sem hardcode
4. **Save/load** — estado do player persistido
