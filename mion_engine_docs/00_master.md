# mion_engine — Master Implementation Document

Master document. Single source of truth for the implementation roadmap.
Indexes all other implementation documents and tracks progress to
**Act 1 complete and round** — story, dialogue, gameplay and action
events all working end-to-end.

> Read this document first. It points to everything else.

---

## Project state

**Engine:** mion_engine (C++17 + SDL3, ARPG narrativo)
**Genre:** ARPG narrativo top-down 2D, pixel art
**References:** Diablo + PoE (gameplay) × Witcher + Disco Elysium (narrative)
**Target:** Act 1 fully implemented. Story, dialogue, gameplay, action
events all complete and playable end-to-end.

### Current capabilities (per CLAUDE.md and end_bug_fix_refactor.md)

- Open world unified (town + dungeon) functional
- Combat system (melee + projectiles + spells) working
- Save system v7, 867 tests passing
- Player progression: XP → level → attribute points + talent points
- Equipment: 11 slots + bag 4×6
- 5 hardcoded spells, 1 talent tree
- Shop system, potion quickslot, dialogue system, locale, audio
- Sprint 5 (current): Actor split into PlayerData/EnemyAIData

### What's being built

- **Karma system** as universal currency (replaces XP + gold)
- **3 skill trees** (warrior, mage, archer) Y-shaped, full freedom
- **3 universal weapons** (sword, bow, magic) with karma-based progression
- **3 traje lines** (armor) evolving with karma
- **Act 1 narrative** — 8 zones, 6 recruitable allies, full storyline
- **City of São Chico** as the open-world map
- **Bilingual** PT-BR / EN dialogues (locale infra exists in `core/locale`)
- **Action events** — choice consequences, hub defense waves, boss phases,
  scripted story moments

### Out of scope for this phase

Polish, custom art, music, Steam integration, marketing — all explicitly
deferred. **Procedural/placeholder assets** during this phase. Engine
already has `tools/` for procedural generation. Focus is gameplay
correctness, not visual fidelity.

---

## Document index

| Document                         | Status | Purpose                                              |
|----------------------------------|--------|------------------------------------------------------|
| `00_master.md` (this doc)        | active | Master index, roadmap, progress tracker              |
| `karma_system_design.md`         | done   | Mechanical design (karma, skills, weapons, traje)    |
| `ato1_historia.md`               | done   | Narrative design (story, characters, zones, tone)    |
| `01_implementation_plan.md`      | todo   | Detailed technical implementation per sprint        |
| `02_skill_tree_definitive.md`    | todo   | All skill nodes for 3 trees (warrior/mage/archer)   |
| `03_zones_content.md`            | todo   | Per zone: enemies, mini-bosses, NPCs, documents, quests, action events |
| `04_dialogues.md`                | todo   | All Act 1 dialogues, bilingual, with localization keys |

---

## Architecture decisions (locked)

### Currency
- **Karma total** = level (never decreases)
- **Karma available** = spendable resource
- No XP, no gold, no separate crafting material

### Progression
- 3 skill trees (warrior/mage/archer), Y-shape (common trunk → 2 branches)
- 3 universal weapons (sword/bow/magic), tier progression by karma
- 3 traje lines (warrior/mage/archer style), tier progression by karma
- Attributes: 5 attributes, focus-per-level (player picks 1, others +1)
- Free respec at altars of dissolution (friction is travel + reinvestment)

### World
- City of São Chico, 8 zones in Act 1
- Difficulty: fixed-tier with floor and ceiling per zone
- Karma drop proportional to effective enemy tier
- Karma-tier-based world reactions (NPCs and visual changes by faixa)

### Narrative
- Protagonist: Dante, 27, terceirizado dev
- Family trigger: Dona Célia + Seu Hélio corrupted, Dante absorbs
- Main goal Act 1: cure parents → discover karma source
- Boss: O Conselheiro — Dante absorbs his karma, learns the truth
- 6 recruitable allies (Diablo-mercenary style)
- 3 hard choices with consequence (faction, hub defense, boss absorption)
- Tone: hybrid (dry/poetic) per character
- Bilingual: PT-BR + EN

### Tech
- C++17, SDL3, header-heavy, namespace `mion`
- INI for game data, code is authority
- Tests are contract: new code → new tests
- Save migration: v7 → v8 (karma) → v9+ (subsequent)
- Localization via existing `core/locale`

### Production approach

- Procedural/placeholder assets — no custom art or music this phase
- Intercalated A + B sprints — implement mechanic, validate with content,
  next mechanic
- Test-driven — every system change has tests before considered done

---

## Roadmap

The path is divided into **2 macro-phases**, intercalated. Mechanic
sprints (K) and content sprints (C) alternate so each system is validated
with real content as it's built.

### Phase A — Karma core systems

| Sprint | Goal | Key sub-sprints |
|--------|------|-----------------|
| Sprint 5 (current) | Actor split | PlayerData + EnemyAIData (per CLAUDE.md) |
| K1 | Karma foundation | KarmaData, drops, level by total, focus attributes, HUD, save v8 |
| K2 | 3 skill trees | INI structure, unlock by karma, Y-branch, synergies, save |
| K3 | Universal weapons | 3 weapons, tier by karma, combat integration, save |
| K4 | Traje system | 3 lines, tier evolution, build sinergy, save |
| K5 | World services + respec | Service NPCs, altar dissolution, heal, buff, potions, map |
| K6 | Zone scaling | Floor/ceiling per zone, scaled drops |
| K7 | Remove legacy | XP, gold, equip slots, shop |
| K8 | Karma faixa consequences | NPC dialogue variations, faixa flag for visual reactions later |

### Phase B — Act 1 content

| Sprint | Goal | Key sub-sprints |
|--------|------|-----------------|
| C1 | São Chico blockout | 8 zone areas wired in WorldMap, zone transitions |
| C2 | Vila Rosário + Centro Velho | Zone 0 base + Zone 1 enemies + O Gerente |
| C3 | Parque + Industrial | Zone 2 + O Jardineiro + Zone 3 + A Máquina |
| C4 | Morro da Vigília hub | Zone 4 NPCs (Marta, Zé, Inácio), barricades, altar |
| C5 | Avenida Progresso + Rio Turvão | Zone 5 + O Síndico + Zone 6 + O Engenheiro |
| C6 | Torre Horizonte + final boss | Zone 7 multi-floor dungeon + O Conselheiro |
| C7 | Allies system | 6 recruitable companions, follow AI, combat behavior |
| C8 | Action events + choices | Faction (Z5), Hub defense waves (Z4), Conselheiro absorption (Z7), scripted story moments |
| C9 | Dialogues integration | All dialogues from `04_dialogues.md` wired, bilingual switching |

### Intercalation order

```
Sprint 5 (Actor split — current)
  ↓
K1 (karma foundation) ───→ C1 (map blockout) ───→ C2 (Vila + Centro)
  ↓
K2 (skill trees) ──────→ K3 (weapons) ──────→ C3 (Parque + Industrial)
  ↓
K4 (traje) ────────→ K5 (services) ────→ C4 (Morro hub)
  ↓
K6 (scaling) ──→ K7 (remove legacy) ──→ C5 (Avenida + Rio)
  ↓
K8 (faixa) ────────────────────────────→ C6 (Torre)
  ↓
C7 (allies) ────→ C8 (events + choices) ────→ C9 (dialogues)
  ↓
ACT 1 COMPLETE
```

Rationale: K1 first (foundation), then C1+C2 to validate karma drop and
combat with real zones. K2-K3 add depth, validated in C3. By K5 the hub
(C4) is essential. K6-K7 cleanup before content scales up. K8 last because
it's content-dependent. C7-C9 close out — allies need all systems
working, choices need allies to react, dialogues need choices to branch.

---

## Progress tracker

Update at end of each sprint.

### Phase A: Karma core systems

- [x] Sprint 5 — Actor split (PlayerData + EnemyAIData em Actor, 4/4 testes, 2026-05-04)
- [x] Sprint K1 — Karma foundation (KarmaData/drop/level/focus/HUD/save v8, 6/6 sub-sprints, 2026-05-23)
- [ ] Sprint K2 — 3 skill trees
- [ ] Sprint K3 — Universal weapons
- [ ] Sprint K4 — Traje system
- [ ] Sprint K5 — World services + respec
- [ ] Sprint K6 — Zone scaling
- [ ] Sprint K7 — Remove legacy systems
- [ ] Sprint K8 — Karma faixa consequences

### Phase B: Act 1 content

- [ ] Sprint C1 — Map blockout
- [ ] Sprint C2 — Vila Rosário + Centro Velho
- [ ] Sprint C3 — Parque + Industrial
- [ ] Sprint C4 — Morro da Vigília hub
- [ ] Sprint C5 — Avenida Progresso + Rio Turvão
- [ ] Sprint C6 — Torre Horizonte + boss
- [ ] Sprint C7 — Allies system
- [ ] Sprint C8 — Action events + choices
- [ ] Sprint C9 — Dialogues integration

### Milestone: Act 1 complete

- [ ] All 8 zones playable end-to-end
- [ ] All karma mechanics functional (skills, weapons, traje, services)
- [ ] All 6 allies recruitable and functional in combat
- [ ] All 7 mini-bosses defeatable (1 per zone except hub)
- [ ] Final boss (O Conselheiro) defeats opens Act 1 ending
- [ ] All dialogues integrated and bilingual switching works
- [ ] All 3 major choices have visible consequences
- [ ] All scripted action events fire correctly
- [ ] No regressions in existing test suite
- [ ] Save migration v7 → vN works across all phase changes

---

## How to use this document

### For Dante (project owner)

- Update progress tracker after each sprint closes
- Use as briefing when starting a new session
- When another doc conflicts with this one, **update this one to match** —
  this doc is the index, not authoritative on details

### For AI assistants in future sessions

When starting a session:
1. Read `CLAUDE.md` (engine contract — non-negotiable)
2. Read `README.md` (current engine state)
3. Read `00_master.md` (this doc — current progress)
4. Read the **specific document** for the sprint at hand
   (e.g., for skill tree work: `02_skill_tree_definitive.md`)
5. Follow the 5-step sprint process from CLAUDE.md
   (brief → plan → approval → implementation → diff review)

### Document hierarchy

```
CLAUDE.md                          ← engine contract (rules, conventions)
  └── README.md                    ← human reference (current state)
       └── 00_master.md            ← THIS DOC (roadmap, progress)
            ├── karma_system_design.md     ← mechanical design (foundation)
            ├── ato1_historia.md           ← narrative design (foundation)
            ├── 01_implementation_plan.md  ← technical execution detail
            ├── 02_skill_tree_definitive.md
            ├── 03_zones_content.md
            └── 04_dialogues.md
```

When code and docs disagree → **code wins**. When docs disagree among
themselves → escalate to Dante.

---

## Sprint discipline (from CLAUDE.md)

Every sprint follows the 5-step process:

1. **Brief** — Dante describes goal in chat
2. **Plan** — IA writes the plan (files read, files written, risks, tests, DONE criteria)
3. **Approval** — Dante OKs / adjusts
4. **Implementation** — code + tests together
5. **Diff review** — Dante approves or asks rework

Non-negotiable rules from CLAUDE.md:
1. Never delete tests to make them pass
2. No TODO/HACK/`#if 0` comments
3. No silenced warnings via cast
4. No mutation of `g_player_config`/`g_spell_defs`/`g_talent_nodes` outside init
5. Code change → matching doc update in same commit
6. Never create new `.md` without explicit request
7. No speculative abstraction
8. Search first before creating new controller/system
9. When in doubt, ASK — don't assume
10. Teach, don't just execute — show trade-offs and wait for decisions

---

## Glossary (PT-BR / EN)

| Term            | PT-BR / EN              | Notes                                  |
|-----------------|-------------------------|----------------------------------------|
| Karma           | karma / karma           | Universal currency. Untranslated.      |
| Traje           | traje / outfit          | Player armor that evolves              |
| Faixa           | faixa / band            | Karma level bracket                    |
| Altar de dissolução | altar de dissolução / dissolution altar | Respec point in the world |
| São Chico       | São Chico               | The city. Untranslated.                |
| Vila Rosário    | Vila Rosário            | Dante's home neighborhood. Untranslated |
| Morro da Vigília | Morro da Vigília       | Hub neighborhood. Untranslated         |
| Dante           | Dante                   | Protagonist. Untranslated.             |
| Dona Célia, Seu Hélio, Lia | (untranslated)| Family. Proper nouns.                  |
| Padre Inácio, Dona Marta, Zé Boticário, Dra. Marina | (untranslated) | NPC names. |
| O Gerente, O Jardineiro, A Máquina, O Síndico, O Engenheiro, O Conselheiro | (untranslated, capitalized title) | Mini-boss / boss titles. |

Localization rules in detail in `04_dialogues.md`.

---

## Open decisions tracker

Decisions still pending. Each resolves before its sprint begins.

### Balancing decisions (resolve during sprints K6-C9)

- [ ] Cost per service tier (heal, buff, potion, map reveal)
- [ ] Floor/ceiling values per zone (provisional table in `karma_system_design.md`)
- [ ] Karma drop curve per enemy tier
- [ ] Skill node costs (provisional in `02_skill_tree_definitive.md`)
- [ ] Weapon tier costs (provisional in `01_implementation_plan.md`)
- [ ] Traje tier costs (provisional in `01_implementation_plan.md`)

### Content decisions (resolve during phase B)

- [ ] Allied combat AI behavior (passive/aggressive toggles?)
- [ ] Hub defense wave structure (number of waves, enemies per wave, time)
- [ ] Faction negotiation outcome (Z5 — what does the player gain/lose precisely)

---

## Next action

Start **Sprint K1 — Karma foundation** after Sprint 5 (Actor split) closes.

Before starting K1, read:
- This doc (overview)
- `karma_system_design.md` §"Karma economy"
- `01_implementation_plan.md` §"Sprint K1" (when written)
