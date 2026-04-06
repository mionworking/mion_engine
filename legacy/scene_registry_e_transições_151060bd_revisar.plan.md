---
name: Scene registry e transições
overview: "Passo 0: refatorar posse de cena e introduzir contexto/registro; em seguida estender portas com destino de cena opcional e ligarum o loop principal a `next_scene()`. Inclui arquivos concretos e o que inserir em cada um."
todos:
  - id: step0-scene-ownership
    content: Refatorar SceneManager para unique_ptr + incluir scene_context.hpp e scene_registry.hpp + register_scenes.cpp em src/core/
    status: completed
  - id: step0-main-wire
    content: "Atualizar main.cpp: registry, create inicial, consumir next_scene a cada fixed step + context (stress via ctx)"
    status: completed
  - id: step1-door-target
    content: Estender DoorZone/add_door e RoomFlowSystem; DungeonScene preenche _pending_next_scene e integra com fluxo existente
    status: completed
  - id: step2-stub-tests
    content: "Opcional: TitleScene ou destino dungeon↔dungeon; testes registry/manager em tests/test_main.cpp"
    status: completed
isProject: false
---

