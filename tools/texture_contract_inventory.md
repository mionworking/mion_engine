# Texture Contract Inventory

Gerado automaticamente por `tools/audit_texture_contract.py`.

- Referências no código: **23**
- Entradas no manifesto: **29**
- Em falta no manifesto: **0**
- Extras no manifesto (não referenciados no código): **6**

- Entradas no backlog planejado: **15**
- Backlog já referenciado no código: **0**
- Backlog ainda não referenciado: **15**

## Referências do código

- `assets/Puny-Characters/Puny-Characters/Mage-Cyan.png`
- `assets/Puny-Characters/Puny-Characters/Mage-Purple.png`
- `assets/Puny-Characters/Puny-Characters/Orc-Grunt.png`
- `assets/Puny-Characters/Puny-Characters/Soldier-Blue.png`
- `assets/Puny-Characters/Puny-Characters/Soldier-Red.png`
- `assets/npcs/npc_elder.png`
- `assets/npcs/npc_forge.png`
- `assets/npcs/npc_mira.png`
- `assets/npcs/npc_villager.png`
- `assets/props/altar.png`
- `assets/props/barrel.png`
- `assets/props/building_elder.png`
- `assets/props/building_forge.png`
- `assets/props/building_tavern.png`
- `assets/props/door_closed.png`
- `assets/props/door_open.png`
- `assets/props/dungeon_pillar.png`
- `assets/props/dungeon_wall_mid.png`
- `assets/props/fountain.png`
- `assets/sprites/player.png`
- `assets/tiles/corridor_floor.png`
- `assets/tiles/dungeon_tileset.png`
- `assets/tiles/town_tileset.png`

## Em falta no manifesto

- Nenhuma

## Extras no manifesto

- `assets/Puny-Characters/Puny-Characters/Archer.png`
- `assets/Puny-Characters/Puny-Characters/Boss-Grimjaw.png`
- `assets/Puny-Characters/Puny-Characters/Elite-Skeleton.png`
- `assets/Puny-Characters/Puny-Characters/Ghost.png`
- `assets/Puny-Characters/Puny-Characters/Patrol-Guard.png`
- `assets/npcs/npc_civil.png`

## Backlog planejado (v2)


### tiles_floor_dungeon
- `assets/tiles/dungeon_floor_base.png` (32x32) — `planned`
- `assets/tiles/dungeon_floor_crack.png` (32x32) — `planned`
- `assets/tiles/dungeon_floor_moss.png` (32x32) — `planned`

### tiles_doors
- `assets/tiles/door_closed.png` (32x48) — `planned`
- `assets/tiles/door_open.png` (32x48) — `planned`
- `assets/tiles/door_locked.png` (32x48) — `planned`

### spell_fx
- `assets/fx/spell_frost_sheet.png` (256x64) — `planned`
- `assets/fx/spell_chain_sheet.png` (256x64) — `planned`
- `assets/fx/spell_nova_sheet.png` (256x128) — `planned`

### projectiles_and_ui_vfx
- `assets/fx/arrow_sheet.png` (128x32) — `planned`
- `assets/fx/hit_flash_sheet.png` (128x64) — `planned`
- `assets/ui/upgrade_icons.png` (192x64) — `planned`

### room_obstacle_variants
- `assets/props/barrel_broken.png` (64x64) — `planned`
- `assets/props/altar_corrupted.png` (80x80) — `planned`
- `assets/props/pillar_cracked.png` (96x128) — `planned`

## Nota de escopo atual

No estado atual do código, **chão, portas e magias** não dependem de texturas PNG dedicadas;
eles são renderizados via fallback geométrico/retângulos/sistemas de efeito. Se forem adicionados
novos caminhos `assets/...png` no código, este relatório passa a listá-los automaticamente.

